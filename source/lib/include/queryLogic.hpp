#pragma once

#include <array>
#include <format>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "GDriveCpp/queryBuilder.h"

namespace GDrive {
    std::string ComparisonOperatorToString(QueryBuilder::ComparisonOperator op) {
        static constexpr std::array<std::string_view, 10> operators = {
            "=", "!=", ">", "<", ">=", "<=", "contains", "in", "starts_with", "ends_with"};
        if (op < QueryBuilder::ComparisonOperator::Equal || op > QueryBuilder::ComparisonOperator::EndsWith) return "";
        return std::string(operators[static_cast<size_t>(op)]);
    }

    struct Condition {
        std::string field;
        QueryBuilder::ComparisonOperator op;
        std::string value;

        Condition(const std::string& field, QueryBuilder::ComparisonOperator op, const std::string& value)
            : field(field), op(op), value(value) {}

        Condition(const std::string& field, QueryBuilder::ComparisonOperator op, std::vector<std::string> values)
            : field(field), op(op), value("") {
            if (op != QueryBuilder::ComparisonOperator::In) {
                throw std::invalid_argument("Invalid operator for multiple values");
            }
            if (values.empty()) {
                value = "()";
            }
            value = "(";
            for (auto it = values.begin(); it != prev(values.end()); it++) {
                value += std::format("'{}',", *it);
            }
            value += std::format("'{}')", values.back());
        }

        std::string toString() const { return std::format("{} {} '{}'", field, ComparisonOperatorToString(op), value); }
    };

    struct LogicBlock {
        std::vector<std::variant<std::shared_ptr<Condition>, std::shared_ptr<LogicBlock>>> conditions;
        enum class Type { And, Or } type;
        LogicBlock() = default;

        LogicBlock(Type type) : type(type) {}

        void AddCondition(std::shared_ptr<Condition> condition) { conditions.push_back(condition); }

        void AddLogicBlock(std::shared_ptr<LogicBlock> block) { conditions.push_back(block); }

        std::string toString() const {
            std::string result;
            std::vector<std::string> conditionStrings;
            for (const auto& cond : conditions) {
                if (std::holds_alternative<std::shared_ptr<Condition>>(cond)) {
                    std::string s = std::get<std::shared_ptr<Condition>>(cond)->toString();
                    if (s.empty()) continue;
                    conditionStrings.emplace_back(s);
                } else if (std::holds_alternative<std::shared_ptr<LogicBlock>>(cond)) {
                    std::string s = std::get<std::shared_ptr<LogicBlock>>(cond)->toString();
                    if (s.empty()) continue;
                    conditionStrings.emplace_back(s);
                }
            }

            if (conditionStrings.empty()) return "";
            if (conditionStrings.size() == 1) {
                return conditionStrings[0];
            }
            result += "(";
            for (size_t i = 0; i < conditionStrings.size() - 1; ++i) {
                result += std::format("{} {} ", conditionStrings[i], type == Type::And ? "and" : "or");
            }
            result += conditionStrings.back() + ")";
            return result;
        }
    };
}  // namespace GDrive