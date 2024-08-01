#pragma once

namespace ethelo
{
    template<typename Ty>
    class json_serializer : public serializer_impl<Ty, json_serializer<Ty>>
    {
    public:
        Ty deserialize(const std::string& text);
        std::string serialize(const Ty& obj);
        static std::string name() { return "json"; }
    };
}
