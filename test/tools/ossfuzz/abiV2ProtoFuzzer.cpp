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

#include <test/tools/ossfuzz/SolidityEvmOneInterface.h>
#include <test/tools/ossfuzz/protoToAbiV2.h>

#include <src/libfuzzer/libfuzzer_macro.h>

#include <fstream>

using namespace solidity::test::fuzzer;
using namespace solidity::test::abiv2fuzzer;
using namespace solidity::test;
using namespace solidity::util;
using namespace solidity;
using namespace std;

static evmc::VM evmone = evmc::VM{evmc_create_evmone()};
/// Expected output value is decimal 0
static vector<uint8_t> const expectedOutput(32, 0);

DEFINE_PROTO_FUZZER(Contract const& _input)
{
	string contract_source = ProtoConverter{}.contractToString(_input);

	if (const char* dump_path = getenv("PROTO_FUZZER_DUMP_PATH"))
	{
		// With libFuzzer binary run this to generate the solidity source file x.sol from a proto input:
		// PROTO_FUZZER_DUMP_PATH=x.sol ./a.out proto-input
		ofstream of(dump_path);
		of << contract_source;
	}

	// Raw runtime byte code generated by solidity
	bytes byteCode;
	std::string hexEncodedInput;

	try
	{
		// Compile contract generated by the proto fuzzer
		SolidityCompilationFramework solCompilationFramework;
		std::string contractName = ":C";
		byteCode = solCompilationFramework.compileContract(contract_source, contractName);
		Json::Value methodIdentifiers = solCompilationFramework.getMethodIdentifiers();
		// We always call the function test() that is defined in proto converter template
		hexEncodedInput = methodIdentifiers["test()"].asString();
	}
	// Ignore stack too deep errors during compilation
	catch (evmasm::StackTooDeepException const&)
	{
		return;
	}
	// Do not ignore other compilation failures
	catch (Exception const&)
	{
		throw;
	}

	if (const char* dump_path = getenv("PROTO_FUZZER_DUMP_CODE"))
	{
		ofstream of(dump_path);
		of << toHex(byteCode);
	}

	// We target the default EVM which is the latest
	langutil::EVMVersion version = {};
	EVMHost hostContext(version, evmone);

	// Deploy contract and signal failure if deploy failed
	evmc::result createResult = EVMOneUtility::deployContract(hostContext, byteCode);
	solAssert(
		createResult.status_code == EVMC_SUCCESS,
		"Proto ABIv2 Fuzzer: Contract creation failed"
	);

	// Execute test function and signal failure if EVM reverted or
	// did not return expected output on successful execution.
	evmc::result callResult = EVMOneUtility::executeContract(
		hostContext,
		fromHex(hexEncodedInput),
		createResult.create_address
	);

	// We don't care about EVM One failures other than EVMC_REVERT
	solAssert(callResult.status_code != EVMC_REVERT, "Proto ABIv2 fuzzer: EVM One reverted");
	if (callResult.status_code == EVMC_SUCCESS)
		solAssert(
			EVMOneUtility::isOutputExpected(callResult.output_data, callResult.output_size, expectedOutput),
			"Proto ABIv2 fuzzer: ABIv2 coding failure found"
		);
}
