#pragma once

#include <optional>
#include <functional>

namespace cn::multi_type_map
{

class ItemSubscription
{
	using SubscriptionType = std::optional<std::function<void()>>;

public:
	ItemSubscription( const std::function<void()>& unsubscribeFunc ) :
		m_UnsubscribeFunction( unsubscribeFunc )
	{}

	/// <summary>Copy construction is deleted to avoid multiple calls to the unsubscribe function</summary>
	ItemSubscription( const ItemSubscription& other ) = delete;

	/// <summary>Copy assignment is deleted to avoid multiple calls to the unsubscribe function</summary>
	ItemSubscription& operator=( const ItemSubscription& other ) = delete;

	ItemSubscription& operator=( ItemSubscription&& other )
	{
		m_UnsubscribeFunction = std::move( other.m_UnsubscribeFunction );

		other.Release();

		return *this;
	}

	void Unsubscribe()
	{
		if ( m_UnsubscribeFunction.has_value() )
		{
			// call to unsubscribe
			m_UnsubscribeFunction.value()();

			Release();
		}
	}

private:

	void Release()
	{
		// remove function pointer, so that we don't try to unsibscribe twice
		m_UnsubscribeFunction = std::nullopt;
	}

	SubscriptionType m_UnsubscribeFunction;
};

}
