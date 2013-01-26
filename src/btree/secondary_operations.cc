// Copyright 2010-2013 RethinkDB, all rights reserved.
#include "btree/operations.hpp"
#include "btree/secondary_operations.hpp"
#include "buffer_cache/blob.hpp"

void get_secondary_indexes_internal(transaction_t *txn, buf_lock_t *sindex_block, std::map<uuid_u, secondary_index_t> *sindexes_out) {
    const btree_sindex_block_t *data = static_cast<const btree_sindex_block_t *>(sindex_block->get_data_read());

    blob_t sindex_blob(const_cast<char *>(data->sindex_blob), btree_sindex_block_t::SINDEX_BLOB_MAXREFLEN);

    buffer_group_t group;
    blob_acq_t acq;
    sindex_blob.expose_all(txn, rwi_read, &group, &acq);

    int64_t group_size = group.get_size();
    std::vector<char> sindex(group_size);

    buffer_group_t group_cpy;
    group_cpy.add_buffer(group_size, sindex.data());

    buffer_group_copy_data(&group_cpy, const_view(&group));

    vector_read_stream_t read_stream(&sindex);
    int res = deserialize(&read_stream, sindexes_out);
    guarantee_err(res == 0, "corrupted secondary index.");
}

void set_secondary_indexes_internal(transaction_t *txn, buf_lock_t *sindex_block, const std::map<uuid_u, secondary_index_t> &sindexes) {
    btree_sindex_block_t *data = static_cast<btree_sindex_block_t *>(sindex_block->get_data_major_write());

    blob_t sindex_blob(data->sindex_blob, btree_sindex_block_t::SINDEX_BLOB_MAXREFLEN);
    sindex_blob.clear(txn);

    write_message_t wm;
    wm << sindexes;

    vector_stream_t stream;
    int res = send_write_message(&stream, &wm);
    guarantee(res == 0);

    sindex_blob.append_region(txn, stream.vector().size());
    std::string sered_data(stream.vector().begin(), stream.vector().end());
    sindex_blob.write_from_string(sered_data, txn, 0);
}

void initialize_secondary_indexes(transaction_t *txn, buf_lock_t *sindex_block) {
    btree_sindex_block_t *data = static_cast<btree_sindex_block_t *>(sindex_block->get_data_major_write());
    memset(data->sindex_blob, 0, btree_sindex_block_t::SINDEX_BLOB_MAXREFLEN);

    blob_t sindex_blob(data->sindex_blob, btree_sindex_block_t::SINDEX_BLOB_MAXREFLEN);

    set_secondary_indexes_internal(txn, sindex_block, std::map<uuid_u, secondary_index_t>());
}

bool get_secondary_index(transaction_t *txn, buf_lock_t *sindex_block, uuid_u uuid, secondary_index_t *sindex_out) {
    std::map<uuid_u, secondary_index_t> sindex_map;

    get_secondary_indexes_internal(txn, sindex_block, &sindex_map);

    std::map<uuid_u, secondary_index_t>::const_iterator it = sindex_map.find(uuid);
    if (it != sindex_map.end()) {
        *sindex_out = it->second;
        return true;
    } else {
        return false;
    }
}

void get_secondary_indexes(transaction_t *txn, buf_lock_t *sindex_block, std::map<uuid_u, secondary_index_t> *sindexes_out) {
    get_secondary_indexes_internal(txn, sindex_block, sindexes_out);
}

void set_secondary_index(transaction_t *txn, buf_lock_t *sindex_block, uuid_u uuid, const secondary_index_t &sindex) {
    std::map<uuid_u, secondary_index_t> sindex_map;
    get_secondary_indexes_internal(txn, sindex_block, &sindex_map);

    /* We insert even if it already exists overwriting the old value. */
    sindex_map[uuid] = sindex;
    set_secondary_indexes_internal(txn, sindex_block, sindex_map);
}

bool delete_secondary_index(transaction_t *txn, buf_lock_t *sindex_block, uuid_u uuid) {
    std::map<uuid_u, secondary_index_t> sindex_map;
    get_secondary_indexes_internal(txn, sindex_block, &sindex_map);

    if (sindex_map.erase(uuid) == 1) {
        set_secondary_indexes_internal(txn, sindex_block, sindex_map);
        return true;
    } else {
        return false;
    }
}

