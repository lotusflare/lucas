INSERT INTO testing.data (
    operator_name,
    id,
    id_type,
    asset_id,
    asset_type,
    approval_status,
    list,
    misc
) VALUES (
    'avantel',
    '9380816255dc45dfa1a57541db81df1d',
    1,
    015e3714-a98b-11ec-9f51-0242ac150008,
    1,
    2,
    [1, 2, 3],
    {
        'car': 'Honda',
        'fruit': 'apple'
    }
) IF NOT EXISTS;

INSERT INTO testing.collections (
    id,
    int_set,
    tinyint_list,
    uuid_to_text_map
) VALUES (
    2,
    {50},
    [10, 20],
    {
        0f854625-9bfd-4407-8c28-c4cdb67106f8: 'foo',
        e00e3562-9b62-40dc-ac40-0b6834ffc5e5: 'bar'
    }
) IF NOT EXISTS;
