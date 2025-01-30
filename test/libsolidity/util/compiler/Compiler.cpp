#include <test/libsolidity/util/compiler/Compiler.h>

using namespace solidity;
using namespace solidity::frontend::test;

EventSignature EventSignature::fromEvent(EventDefinition const& _event)
{
	auto externalSignature = _event.functionType(true)->externalSignature();
	bool isAnonymous = _event.isAnonymous();

	auto toString = [](auto const& _param) { return _param->type()->toString(true); };

	auto indexedTypes =
		_event.parameters() |
		ranges::views::filter([](auto const& _param) { return _param->isIndexed(); }) |
		ranges::views::transform(toString) |
		ranges::to<std::vector>();
	auto nonIndexedTypes =
		_event.parameters() |
		ranges::views::filter([](auto const& _param) { return !_param->isIndexed(); }) |
		ranges::views::transform(toString) |
		ranges::to<std::vector>();

	return EventSignature{
		externalSignature,
		indexedTypes,
		nonIndexedTypes,
		isAnonymous
	};
}

CompiledContract const* CompilerOutput::contract(ContractName const& _name) const
{
    auto const& sourceName = std::string{_name.source()};
    auto const& contractName = std::string{_name.contract()};

    if (
    	auto const& source = m_sourceUnits.find(sourceName);
     	source != m_sourceUnits.end()
    )
    {
        if (!contractName.empty())
        {
            for (auto const& contract: source->second)
                if (
                    contractName == contract.name ||
                    sourceName + ":" + contractName == contract.name
                )
                    return &contract;
        }
        else
        {
            if (!source->second.empty())
                return &source->second.back();
        }
    }

    return nullptr;
}

EventSignature const* CompilerOutput::matchEvent(util::h256 const& _hash) const
{
    for (auto const& contracts: m_sourceUnits | ranges::views::values)
        for (auto const& contract: contracts)
            for (auto const& event: contract.eventSignatures)
                if (!event.isAnonymous && keccak256(event.signature) == _hash)
                    return &event;

    return nullptr;
}

bool CompilerOutput::success() const
{
    return m_success;
}

std::optional<langutil::Error> CompilerOutput::findError(
    langutil::Error::Type _type
) const
{
    for (auto const& error: m_errors)
		if (error->type() == _type)
			return std::make_optional(*error);

    return std::nullopt;
}

std::string_view CompilerOutput::errorInformation() const
{
    return m_errorInformation;
}
