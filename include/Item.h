#pragma once

#include <memory>
#include <atomic>
#include <functional>
#include <mutex>
#include <list>

#include "ItemSubscription.h"

namespace cn::multi_type_map
{

struct DefaultItemKey;

template <class _Ty, class _Key>
class Item
{
public:

	using ItemType = Item<_Ty, _Key>;

	using ObserverFunction = std::function<void( const _Ty& )>;
	using ObserverContainer = std::list<ObserverFunction>;
	using ObserverIterator = typename ObserverContainer::iterator;

public:

	std::shared_ptr<_Ty> Get() const
	{
		return std::atomic_load( &m_Value );
	}

	void Set( const std::shared_ptr<_Ty>& value )
	{
		std::atomic_exchange( &m_Value, value );

		NotifyObservers();
	}

	// todo: this could be problematic if this is deleted and unique_ptr is being held by caller
	// fix: could return weak_ptr here
	[[nodiscard]]
	std::unique_ptr<ItemSubscription> Subscribe( const ObserverFunction& observer )
	{
		std::unique_lock lockObservers( m_ObserverMutex );

		m_Observers.emplace_back( observer );

		auto it = std::prev( std::end( m_Observers ) );

		// this assumes that the lifetime of this class is longer than the subscriber
		return std::make_unique<ItemSubscription>(
			[this, it]()
		{
			this->Unsubscribe( it );
		} );
	}

	void Unsubscribe( ObserverIterator it )
	{
		std::unique_lock lockObservers( m_ObserverMutex );

		m_Observers.erase( it );
	}

protected:

	void NotifyObservers()
	{
		std::unique_lock lockObservers( m_ObserverMutex );

		for ( auto& observer : m_Observers )
		{
			if ( observer )
				observer( *m_Value );
		}
	}

	std::shared_ptr<_Ty> m_Value;

	std::mutex m_ObserverMutex;

	ObserverContainer m_Observers;
};

}