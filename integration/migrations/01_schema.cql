CREATE KEYSPACE IF NOT EXISTS testing WITH replication = {
    'class': 'SimpleStrategy',
    'replication_factor': 3
};

CREATE TABLE IF NOT EXISTS testing.data(
    operator_name text,
    id text,
    id_type int,
    created_at timestamp,
    asset_id timeuuid,
    asset_type int,
    approval_status int,
    misc map<text, text>,
    list list<int>,
    PRIMARY KEY ((operator_name, id), asset_type)
);

CREATE TABLE IF NOT EXISTS testing.collections(
    id bigint,
    uuid_to_text_map map<uuid, text>,
    int_set set<int>,
    tinyint_list list<tinyint>,
    PRIMARY KEY (id)
);
