#pragma once

#include <memory>
#include <string>
#include <vector>

#include "dllExport.h"

namespace GDrive {
    struct QueryBuilderImpl;

#pragma warning(push)
#pragma warning(disable : 4251)  // Disable C4251 for this class (QueryBuilderImpl is meant not to be exported)

    class GDRIVE_API QueryBuilder {
      public:
        enum class ComparisonOperator {
            Equal = 0,
            NotEqual = 1,
            GreaterThan = 2,
            LessThan = 3,
            GreaterThanOrEqual = 4,
            LessThanOrEqual = 5,
            Contains = 6,
            In = 7,
            StartsWith = 8,
            EndsWith = 9
        };

        QueryBuilder();
        ~QueryBuilder();

        QueryBuilder(const QueryBuilder&) = delete;
        QueryBuilder& operator=(const QueryBuilder&) = delete;

        // Declare move constructor (defined in .cpp, defaulted there)
        [[nodiscard]] QueryBuilder(QueryBuilder&&) noexcept;
        // Declare move assignment operator (defined in .cpp, defaulted there)
        [[nodiscard]] QueryBuilder& operator=(QueryBuilder&&) noexcept;

        QueryBuilder& And();
        QueryBuilder& Or();
        QueryBuilder& EndBlock();

        QueryBuilder& AddCondition(const std::string& field, ComparisonOperator op,
                                                 const std::string& value,
                                   bool enabled = true);
        QueryBuilder& AddCondition(const std::string& field, ComparisonOperator op,
                                   const std::vector<std::string>& values, bool enabled = true);
        [[nodiscard]] std::string Build() const;

      private:
        std::unique_ptr<QueryBuilderImpl> impl;
    };

#pragma warning(pop)  // Re-enable C4251

}  // namespace GDrive
