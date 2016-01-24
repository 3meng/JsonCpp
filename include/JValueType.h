//
// Created by qife on 16/1/11.
//

#ifndef CPPPARSER_JVALUETYPE_H
#define CPPPARSER_JVALUETYPE_H

#include <string>

namespace JsonCpp
{
    enum JValueType
    {
        Null,
        Boolean,
        Number,
        String,
        Object,
        Array
    };

    namespace Expr
    {
        enum ExprType
        {
            Numeric,
            Operator,
            Boolean,
            Property
        };

        union ExprData
        {
            char op;
            bool bv;
            double num;
            std::string *prop;
        };

        struct ExprNode
        {
            ExprType type;
            ExprData data;

            ExprNode(ExprType t = ExprType::Numeric) : type(t) { data.prop = nullptr; }

            ExprNode(ExprNode &&node) : type(node.type)
            {
                switch (node.type)
                {
                    case Numeric:
                        data.num = node.data.num;
                        node.data.num = 0.0;
                        break;

                    case Operator:
                        data.op = node.data.op;
                        node.data.op = '\0';
                        break;

                    case Boolean:
                        data.bv = node.data.bv;
                        node.data.bv = false;
                        break;

                    case Property:
                        data.prop = node.data.prop;
                        node.data.prop = nullptr;
                        break;
                }

                node.type = Numeric;
            }

            ~ExprNode()
            {
                if (type == ExprType::Property && data.prop)
                {
                    delete data.prop;
                    data.prop = nullptr;
                }
            }
        };

        enum BoolOpType
        {
            Greater,
            Less,
            GreaterEqual,
            LessEqual,
            Equal,
            NotEqual
        };
    }
}

#endif //CPPPARSER_JVALUETYPE_H
