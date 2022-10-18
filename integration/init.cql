CREATE KEYSPACE IF NOT EXISTS testing WITH replication = {
    'class': 'SimpleStrategy',
    'replication_factor': 1
};

CREATE TABLE testing.data(
    operator_name text,
    id text,
    id_type int,
    asset_id timeuuid,
    asset_type int,
    approval_status int,
    misc map<text, text>,
    PRIMARY KEY ((operator_name, id), asset_type)
);

INSERT INTO testing.data (operator_name, id, id_type, asset_id, asset_type, approval_status)
VALUES ('avantel', '9380816255dc45dfa1a57541db81df1d', 1, 015e3714-a98b-11ec-9f51-0242ac150008, 1, 2);
