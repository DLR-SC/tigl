#pragma once

#include <boost/optional.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace tigl
{
	template <typename CacheStruct, typename CpacsClass>
	class Cache
	{
	public:
		typedef void(CpacsClass::* BuildFunc)(CacheStruct&) const;

		Cache(CpacsClass& instance, BuildFunc buildFunc)
			: m_instance(instance), m_buildFunc(buildFunc)
		{
		}

		const CacheStruct& value() const
		{
			boost::lock_guard<boost::mutex> guard(m_mutex);
			if (!m_cache) {
				m_cache.emplace();
				(m_instance.*m_buildFunc)(*m_cache);
			}
			return m_cache.value();
		}

		const CacheStruct& operator*() const { return value(); }
		const CacheStruct* operator->() const { return &value(); }

		void clear()
		{
			boost::lock_guard<boost::mutex> guard(m_mutex);
			m_cache = boost::none;
		}

	private:
		CpacsClass& m_instance;
		BuildFunc m_buildFunc;
		mutable boost::mutex m_mutex;
		mutable boost::optional<CacheStruct> m_cache;
	};
}
