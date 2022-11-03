CREATE KEYSPACE IF NOT EXISTS testing WITH replication = {
    'class': 'SimpleStrategy',
    'replication_factor': 1
};

CREATE TABLE IF NOT EXISTS testing.data(
    operator_name text,
    id text,
    id_type int,
    asset_id timeuuid,
    asset_type int,
    approval_status int,
    misc map<text, text>,
    list list<int>,
    PRIMARY KEY ((operator_name, id), asset_type)
);
