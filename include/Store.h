#pragma once

#include "Item.h"
#include <typeindex>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <assert.h>

namespace cn::multi_type_map
{

// Use parameter packing to inherit properties of all items
template<class... Items>
class Store : private Items...
{
	using this_type = Store<Items...>;

public:

	Store() = default;

	Store( const this_type& _other )
	{
		*this = _other;
	}

	Store& operator=( const Store<Items...>& from )
	{
		m_CompareFunctions = from.m_CompareFunctions;

		m_CopyFunctions = from.m_CopyFunctions;

		std::for_each(
			std::begin( m_CopyFunctions ),
			std::end( m_CopyFunctions ),
			[&]( auto&& entry )
		{
			return entry.second( from, *this );
		} );

		return *this;
	}

	bool operator==( const Store<Items...>& rhs ) const
	{
		return std::all_of(
			std::begin( m_CompareFunctions ),
			std::end( m_CompareFunctions ),
			[&]( auto&& entry )
		{
			return entry.second( *this, rhs );
		} );
	}

	bool operator!=( const Store<Items...>& rhs ) const
	{
		return std::any_of(
			std::begin( m_CompareFunctions ),
			std::end( m_CompareFunctions ),
			[&]( auto&& entry )
		{
			return !entry.second( *this, rhs );
		} );
	}

	size_t Size()
	{
		return m_CompareFunctions.size();
	}

	template <class _Ty, class _Key = DefaultItemKey>
	std::shared_ptr<_Ty> Get() const
	{
		static_assert(std::is_base_of<Item<_Ty, _Key>, Store<Items...>>::value, "Invalid Type or Key");

		return Item<_Ty, _Key>::Get();
	}

	template <class _Ty, class _Key = DefaultItemKey>
	void Set( const std::shared_ptr<_Ty>& value )
	{
		static_assert(std::is_base_of<Item<_Ty, _Key>, Store<Items...>>::value, "Invalid Type or Key");

		InitType<_Ty, _Key>();

		Item<_Ty, _Key>::Set( value );
	}

	template <class _Ty, class _Key = DefaultItemKey, class ...Args>
	void Emplace( Args&&... args )
	{
		static_assert(std::is_base_of<Item<_Ty, _Key>, Store<Items...>>::value, "Invalid Type or Key");

		InitType<_Ty, _Key>();

		Item<_Ty, _Key>::Set( std::make_shared<_Ty>( std::forward<Args>( args )... ) );
	}

	template <class _Ty, class _Key = DefaultItemKey>
	[[nodiscard]] std::unique_ptr<ItemSubscription> Subscribe( const std::function<void( const _Ty& )>& func )
	{
		static_assert(std::is_base_of<Item<_Ty, _Key>, Store<Items...>>::value, "Invalid Type or Key");

		return Item<_Ty, _Key>::Subscribe( func );
	}

private:

	template <class _Ty, class _Key>
	void InitType()
	{
		if ( m_CopyFunctions.find( typeid(Item<_Ty, _Key>) ) == m_CopyFunctions.end() ) // if not already there 
		{
			std::type_index typeId = typeid(Item<_Ty, _Key>);

			m_CopyFunctions.insert_or_assign( typeId, []( const this_type& from, this_type& to )
			{
				// todo : the subscribers don't get copied. not sure if that's a good idea
				to.Item<_Ty, _Key>::m_Value = from.Item<_Ty, _Key>::m_Value;

				to.Item<_Ty, _Key>::m_Observers = from.Item<_Ty, _Key>::m_Observers;
			} );

			m_CompareFunctions.insert_or_assign( typeId, []( const this_type& lhs, const this_type& rhs ) -> bool
			{
				return lhs.Item<_Ty, _Key>::m_Value == rhs.Item<_Ty, _Key>::m_Value; // FIX : this is comparing the pointers
			} );

			assert( m_CompareFunctions.size() == m_CopyFunctions.size() );
		}
	}

	std::unordered_map<std::type_index, std::function<void( const this_type&, this_type& )>> m_CopyFunctions;

	std::unordered_map<std::type_index, std::function<bool( const this_type&, const this_type& )>> m_CompareFunctions;
};

}