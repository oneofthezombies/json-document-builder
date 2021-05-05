#include "JsonDocumentBuilder.hpp"

#include <iostream>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

int main() {
    const int32_t variableInt = 3;
    const char* variableCString = "4";
    const std::string variableString = "5";
    const std::vector<int32_t> numberList = { 6, 7 };
    const std::vector<std::string> stringList = { "8", "9" };

    struct MyObject {
        std::string value1;
        std::string value2;
    };

    const std::vector<MyObject> myObjectList = {
        {"16", "17"}, {"18", "19"}
    };

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

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
    document.Accept(writer);
    std::cout << sb.GetString() << std::endl;

    return 0;
}
