#include <format>
#include <iostream>
#include <string>

#include "GDriveCpp/gDrive.h"
#include "GDriveCpp/gFile.h"
#include "GDriveCpp/queryBuilder.h"
#include "google_api_key.hpp"
#include "logging.hpp"

int main() {
    utils::logging::LogConsole logConsole(
        "GDriveTest", 0, [](const std::string& msg) { std::cout << "Log message: " << msg << std::endl; });
    logConsole.create();
    std::string clientId = std::string(secrets::clientId);
    std::string clientSecret = std::string(secrets::clientSecret);
    std::shared_ptr<GCloud::Authentication::OAuthAgent> client =
        std::make_shared<GCloud::Authentication::OAuthAgent>(clientId, clientSecret);
    try {
        std::string token = client->getAccessToken();
        std::cout << "Authentication successful!" << std::endl;
        std::cout << "Token: " << token << std::endl;

        std::string q = GDrive::QueryBuilder()
                            .And()
                                .AddCondition("name", GDrive::QueryBuilder::ComparisonOperator::Equal, "Server1")
                                .AddCondition("field2", GDrive::QueryBuilder::ComparisonOperator::Equal, "value2")
                                .Or()
                                    .AddCondition("field3", GDrive::QueryBuilder::ComparisonOperator::Equal, "value3")
                                    .AddCondition("field4", GDrive::QueryBuilder::ComparisonOperator::Equal, "value4")
                                .EndBlock()
                            .EndBlock()
                            .Build();

        spdlog::info("Query: {}", q);

        GDrive::GFileList list = GDrive::GFileList::QueryDirectory(client, nullptr, "tac/Server1/").value();

        list.print(std::cout);
        if (!list.files.empty()) {
            list.files[0]->download();
        }
    } catch (const std::exception& e) {
        std::cout << "Authentication failed: " << e.what() << std::endl;
    }
    return 0;
}
