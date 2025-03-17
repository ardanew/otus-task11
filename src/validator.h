#pragma once
#include "command.h"
#include <string>
#include <vector>
#include <tuple>

/// \brief struct to hold parsing results
struct ValidateResult
{
	bool ok;
	std::string errorMessage;
	CMD_ID command;
	std::vector<std::string> parameters;
};

/// \brief class to validate data and construct commands
class Validator
{
public:
	/// \brief parses the tokens and constructs command with parameters if parsing was successful
	static ValidateResult parse(const std::vector<std::string> &tokens);

protected:
	static bool validateTableName(const std::string &tableName);
	static ValidateResult parseInsertCommand(const std::vector<std::string> &tokens);
	static ValidateResult parseTruncateCommand(const std::vector<std::string> &tokens);
	static ValidateResult parseIntersectionCommand(const std::vector<std::string> &tokens);
	static ValidateResult parseSymmetricDifferenceCommand(const std::vector<std::string> &tokens);
};