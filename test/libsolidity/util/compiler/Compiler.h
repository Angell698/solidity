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

#include <libevmasm/Assembly.h>

#include <libsolidity/interface/CompilerStack.h>

#include <libsolutil/Common.h>

using namespace solidity;
using namespace solidity::util;

namespace solidity::frontend::test
{

using MetadataFormat = CompilerStack::MetadataFormat;
using MetadataHash = CompilerStack::MetadataHash;

struct EventSignature
{
	static EventSignature fromEvent(EventDefinition const& _event);

	std::string signature;
	std::vector<std::string> indexedTypes;
	std::vector<std::string> nonIndexedTypes;
	bool isAnonymous;
};

template<typename T>
concept StringViewConvertible = std::is_convertible_v<T, std::string_view>;

/**
 * Helper for contract lookups. A `ContractName` can be initialized from
 * either an unqualified contract name like "C" or a qualified name
 * like ":C" or "lib.sol:C".
 */
class ContractName
{
public:
	/// Construct an empty, unqualified contract name
	ContractName(): m_isUnqualified(true) {}

	/// Construct a new contract name by parsing the given name.
	/// @param _name the contract name, e.g. "C", ":C" or "lib.sol:C"
	// explicit ContractName(std::string_view const _name)
	// {

	// }

	/// Support implicit conversion from e.g. string literals.
	template<StringViewConvertible T>
	ContractName(T const& _name)
	{
		auto const [source, contract, isUnqualified] = decompose(_name);
		m_source = source;
		m_contract = contract;
		m_isUnqualified = isUnqualified;
	}

	/// Construct a fully-qualified contract name.
	/// @param _source the source name
	/// @param _contract the contract name
	ContractName(std::string _source, std::string _contract):
		m_source(std::move(_source)),
		m_contract(std::move(_contract)),
		m_isUnqualified(false)
	{}

	static std::tuple<std::string, std::string, bool> decompose(
		std::string_view const _name
	)
	{
		std::string source;
		std::string contract;
		bool isUnqualified;

		auto const pos = _name.find(':');
		if (pos != std::string::npos)
		{
			isUnqualified = false;
			source = _name.substr(0, pos);
			contract = _name.substr(pos + 1);
		}
		else
		{
			isUnqualified = true;
			contract = _name;
		}

		return {source, contract, isUnqualified};
	}

	/// @returns the source name.
	std::string_view const source() const
	{
		return m_source;
	}

	/// @returns the contractname.
	std::string_view const contract() const
	{
		return m_contract;
	}

	/// @returns false if this was initialized from an unqualified contract
	/// name like "C".
	bool isUnqualified() const
	{
		return m_isUnqualified;
	}

private:
	/// The source name. Can be empty.
	std::string m_source;
	/// The contract name. Can be empty.
	std::string m_contract;
	/// True, if this is an unqualified contract name.
	bool m_isUnqualified;
};

struct CompilerInput
{
	/// Source code to be compiled
	std::map<std::string, std::string> sourceCode;
	/// Information on which library is deployed where
	std::map<std::string, solidity::util::h160> libraryAddresses;
	/// Contract name without a colon prefix
	std::optional<std::string> contractName;
	/// EVM target version
	std::optional<langutil::EVMVersion> evmVersion;
	/// EOF version
	std::optional<uint8_t> eofVersion;
	/// If optimisation should be enabled.
	std::optional<bool> optimise;
	/// Optimiser setting to be used during compilation
	std::optional<frontend::OptimiserSettings> optimiserSettings;
	/// Revert string behaviour
	std::optional<RevertStrings> revertStrings;
	/// Format of the metadata appended at the end of the bytecode
	std::optional<MetadataFormat> metadataFormat;
	/// Hash should be used to store the metadata in the bytecode
	std::optional<MetadataHash> metadataHash;
	/// Flag used for debugging
	std::optional<bool> debugFailure;
	/// Flag to enable new code generator
	std::optional<bool> viaIR;
};

/**
 * A compiled contract.
 */
struct CompiledContract
{
	/// The name of the contract
	std::string name;
	/// Bytecode of the assembled object for a contract.
	bytes object;
	/// Bytecode of the assembled runtime object for a contract.
	bytes runtimeObject;
	/// If object has unlinked references.
	bool hasUnlinkedReferences;
	/// CBOR-encoded metadata matching the pipeline selected using the viaIR
	/// setting.
	bytes cborMetadata;
	/// Normal contract assembly items.
	std::optional<evmasm::AssemblyItems> assemblyItems;
	/// Normal contract runtime assembly items.
	std::optional<evmasm::AssemblyItems> runtimeAssemblyItems;
	/// Contract Metadata matching the pipeline selected using the viaIR setting.
	std::optional<std::string> metadata;
	/// The contract ABI as a JSON object.
	std::optional<Json> contractABI;
	/// JSON object with the three members ``methods``, ``events``, ``errors``.
	/// Each is a map, mapping identifiers (hashes) to function names.
	std::optional<Json> interfaceSymbols;
	/// Event signatures
	std::vector<EventSignature> eventSignatures;
};

using SourceUnits = std::map<std::string, std::vector<CompiledContract>>;

/**
 * Output generated by the compiler. Provides convenient access method to the
 * internal data.
 */
class CompilerOutput
{
public:
	CompilerOutput(
		SourceUnits _sourceUnits,
		bool _success,
		langutil::ErrorList _errors,
		std::string _errorInformation
	):
		m_sourceUnits(std::move(_sourceUnits)),
		m_success(_success),
		m_errors(std::move(_errors)),
		m_errorInformation(std::move(_errorInformation))
	{}

	/// @returns the compiled contract, if any was found.
	/// @param _name find specific contract, if set. Default: return last contract
	/// from source with empty name.
	CompiledContract const* contract(ContractName const& _name = {}) const;

	/// @returns the annotated event sig for the hash given, if any.
	/// @param hash
	EventSignature const* matchEvent(h256 const& _hash) const;

	/// @returns true if no errors occurred during compilation.
	bool success() const;

	/// @returns the first error that matches the type specified, or none if no
	/// error for this type was found.
	/// @param _type the error type to look for.
	std::optional<langutil::Error> findError(langutil::Error::Type _type) const;

	/// @returns a formatted output of all errors that occurred during
	/// compilation.
	std::string_view errorInformation() const;

private:
	/// All compiled contracts, indexed by source name.
	SourceUnits m_sourceUnits;
	/// If compilation was successful.
	bool m_success;
	/// All errors, if any.
	langutil::ErrorList m_errors;
	/// Formatted error information.
	std::string m_errorInformation;
};

}
