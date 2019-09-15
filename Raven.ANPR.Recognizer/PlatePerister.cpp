#include "PlatePersister.h"
namespace ravendb::recognizer
{
	PlatePersister::PlatePersister(std::vector<std::string>& urls, const std::string& database_name)
	{
		_documentStore = DocumentStore::create(urls, database_name);
		_documentStore->initialize();
	}

	PlatePersister::~PlatePersister()
	{
		_documentStore->close();
	}

	PlatePersister::PlatePersister(PlatePersister&& other) noexcept: _documentStore(std::move(other._documentStore))
	{
	}

	PlatePersister& PlatePersister::operator=(const PlatePersister& other)
	{
		if (this == &other)
			return *this;
		_documentStore = other._documentStore;
		return *this;
	}

	PlatePersister& PlatePersister::operator=(PlatePersister&& other) noexcept
	{
		if (this == &other)
			return *this;
		_documentStore = std::move(other._documentStore);
		return *this;
	}
}
