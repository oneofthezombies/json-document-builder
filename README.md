# json-document-builder
example
```cpp
// variables are here.
// ...

rapidjson::Document document = json_document_builder::Builder()
    .at("/A").set(nullptr)
    .at("/B").set(false)
    .at("/C").set(0)
    .at("/D").set(0.5)
    .at("/E(%d)", 1).set("(%d)", 2)
    .at("/F").set(variableInt)
    .at("/G").set(variableCString)
    .at("/H").set(variableString)
    .at("/I").set(numberList)
    .at("/J").set(stringList)
    .at("/K").set(std::vector<std::string>{ "10", "11" })
    .at("/L").set(std::vector<int32_t>{ 12, 13 })
    .at("/M").set(json_document_builder::Builder()
        .at("/N").set(14)
        .at("/O").set(15)
        .build())
    .at("/P").set<MyObject>(myObjectList, [](const MyObject& value) {
        return json_document_builder::Builder()
            .at("/P0").set(value.value1)
            .at("/P1").set(value.value2)
            .build();
    })
    .build();
```
result
```json
{
    "A": null,
    "B": false,
    "C": 0,
    "D": 0.5,
    "E(1)": "(2)",
    "F": 3,
    "G": "4",
    "H": "5",
    "I": [
        6,
        7
    ],
    "J": [
        "8",
        "9"
    ],
    "K": [
        "10",
        "11"
    ],
    "L": [
        12,
        13
    ],
    "M": {
        "N": 14,
        "O": 15
    },
    "P": [
        {
            "P0": "16",
            "P1": "17"
        },
        {
            "P0": "18",
            "P1": "19"
        }
    ]
}
```