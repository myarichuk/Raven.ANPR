#pragma once

#ifndef RAVEN_ANPR_PERSISTER_H
#define RAVEN_ANPR_PERSISTER_H

#include <iostream>
#include <string>
#include <DocumentStore.h>

namespace ravendb::recognizer
{
	using namespace client::documents;
	class PlatePersister
	{
	protected:
		std::shared_ptr<DocumentStore> _documentStore;
	public:
		PlatePersister() = delete;	

		PlatePersister(std::vector<std::string>& urls, const std::string& database_name);
		~PlatePersister();

		PlatePersister(const PlatePersister& other) = default;
		PlatePersister(PlatePersister&& other) noexcept;
		PlatePersister& operator=(const PlatePersister& other);
		PlatePersister& operator=(PlatePersister&& other) noexcept;
	};
}


#endif