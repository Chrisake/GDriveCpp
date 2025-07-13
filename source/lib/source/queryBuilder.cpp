#include "GDriveCpp/queryBuilder.h"

#include <memory>
#include <stack>

#include "logging.hpp"
#include "queryLogic.hpp"

namespace GDrive {
    QueryBuilder::QueryBuilder() : impl(std::make_unique<QueryBuilderImpl>()) {}

    QueryBuilder::~QueryBuilder() = default;

    QueryBuilder::QueryBuilder(QueryBuilder&& other) noexcept = default;

    QueryBuilder& QueryBuilder::operator=(QueryBuilder&& other) noexcept = default;

    struct QueryBuilderImpl {
        std::stack<std::shared_ptr<LogicBlock>> logicStack;

        QueryBuilderImpl() : root(new LogicBlock(LogicBlock::Type::And)) { logicStack.push(root); };

        std::shared_ptr<LogicBlock> root;

        void AddCondition(std::shared_ptr<Condition> condition) {
            if (logicStack.empty()) {
                throw std::runtime_error("Logic stack is empty, cannot add condition.");
            }
            logicStack.top()->AddCondition(condition);
        }

        void AddLogicBlock(std::shared_ptr<LogicBlock> block) {
            if (logicStack.empty()) {
                throw std::runtime_error("Logic stack is empty, cannot add logic block.");
            }

            logicStack.top()->AddLogicBlock(block);
            logicStack.push(block);
        }
    };

    QueryBuilder& QueryBuilder::And() {
        impl->AddLogicBlock(std::make_shared<LogicBlock>(LogicBlock::Type::And));
        return *this;
    }

    QueryBuilder& QueryBuilder::Or() {
        impl->AddLogicBlock(std::make_shared<LogicBlock>(LogicBlock::Type::Or));
        return *this;
    }

    QueryBuilder& QueryBuilder::AddCondition(const std::string& field, ComparisonOperator op, const std::string& value,
                                             bool enabled) {
        if (!enabled) {
            return *this;
        }
        auto condition = std::make_shared<Condition>(field, op, value);
        impl->AddCondition(condition);
        return *this;
    }

    QueryBuilder& QueryBuilder::AddCondition(const std::string& field, ComparisonOperator op,
                                             const std::vector<std::string>& values, bool enabled) {
        if (!enabled) {
            return *this;
        }
        auto condition = std::make_shared<Condition>(field, op, values);
        impl->AddCondition(condition);
        return *this;
    }

    QueryBuilder& QueryBuilder::EndBlock() {
        if (impl->logicStack.top().get() == impl->root.get()) {
            throw std::runtime_error(
                "Logic Block Imbalance! Query contains more End Blocks than the blocks themselves.");
        }
        impl->logicStack.pop();
        return *this;
    }

    std::string QueryBuilder::Build() const {
        if (impl->logicStack.size() != 1) {
            spdlog::warn("Logic Block Imbalance! Some Logic Blocks might not have closed");
        }
        return impl->root->toString();
    }
}  // namespace GDrive
