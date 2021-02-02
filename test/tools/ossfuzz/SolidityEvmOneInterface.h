/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <test/EVMHost.h>

#include <libsolidity/interface/CompilerStack.h>

#include <libyul/AssemblyStack.h>

#include <libsolutil/Keccak256.h>

#include <evmone/evmone.h>

namespace solidity::test::fuzzer
{
class SolidityCompilationFramework
{
public:
	explicit SolidityCompilationFramework(langutil::EVMVersion _evmVersion = {});

	Json::Value getMethodIdentifiers()
	{
		return m_compiler.methodIdentifiers(m_compiler.lastContractName());
	}
	bytes compileContract(
		std::string const& _sourceCode,
		std::string const& _contractName,
		std::map<std::string, solidity::util::h160> const& _libraryAddresses = {},
		frontend::OptimiserSettings _optimization = frontend::OptimiserSettings::minimal()
	);
protected:
	frontend::CompilerStack m_compiler;
	langutil::EVMVersion m_evmVersion;
};

struct EVMOneUtility
{
	/// Compares the contents of the memory address pointed to
	/// by `_result` of `_length` bytes to the expected output.
	/// Returns true if `_result` matches expected output, false
	/// otherwise.
	static bool isOutputExpected(
		uint8_t const* _result,
		size_t _length,
		std::vector<uint8_t> const& _expectedOutput
	);
	/// Accepts a reference to a user-specified input and returns an
	/// evmc_message with all of its fields zero initialized except
	/// gas and input fields.
	/// The gas field is set to the maximum permissible value so that we
	/// don't run into out of gas errors. The input field is copied from
	/// user input.
	static evmc_message initializeMessage(bytes const& _input);
	/// Accepts host context implementation, and keccak256 hash of the function
	/// to be called at a specified address in the simulated blockchain as
	/// input and returns the result of the execution of the called function.
	static evmc::result executeContract(
		EVMHost& _hostContext,
		bytes const& _functionHash,
		evmc_address _deployedAddress
	);
	/// Deploys @param _code on @param _hostContext.
	static evmc::result deployContract(EVMHost& _hostContext, bytes const& _code);
	/// Deploys and executes EVM byte code in @param _byteCode on
	/// EVM Host referenced by @param _hostContext. Input passed
	/// to execution context is @param _hexEncodedInput.
	/// @returns result returning by @param _hostContext.
	static evmc::result deployAndExecute(
		EVMHost& _hostContext,
		bytes _byteCode,
		std::string _hexEncodedInput
	);
	/// Compiles/deploys @param _contractName in @param _sourceCode
	/// and executes @param _methodName via @param _host. Optimisation
	/// setting @param _optimization is used to choose the desired
	/// optimisation. Optionally library named @param _libraryName
	/// may be referenced/pre deployed.
	static evmc::result compileDeployAndExecute(
		EVMHost& _host,
		std::string _sourceCode,
		std::string _contractName,
		std::string _methodName,
		frontend::OptimiserSettings _optimisation,
		std::string _libraryName = {}
	);
	/// Compiles contract named @param _contractName present in
	/// @param _sourceCode, optionally using a precompiled library
	/// specified via a library mapping and an optimisation setting.
	/// @returns a pair containing the generated byte code and method
	/// identifiers for methods in @param _contractName.
	static std::pair<bytes, Json::Value> compileContract(
		std::string _sourceCode,
		std::string _contractName,
		std::map<std::string, solidity::util::h160> const& _libraryAddresses = {},
		frontend::OptimiserSettings _optimisation = frontend::OptimiserSettings::minimal()
	);
};

}
