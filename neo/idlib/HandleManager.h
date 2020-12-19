#pragma once

#include <stdint.h>

constexpr uint16_t kInvalidHandle = UINT16_MAX;

class HandleManager
{
public:
	HandleManager( uint16_t aMaxHandles )
		: m_numHandles( 0 )
		, m_maxHandles( aMaxHandles )
	{
		reset();
	}

	~HandleManager()
	{
	}

	const uint16_t* getHandles() const
	{
		return ( const uint16_t* )getDensePtr();
	}

	uint16_t getHandleAt( uint16_t aAt ) const
	{
		return getDensePtr()[aAt];
	}

	uint16_t getNumHandles() const
	{
		return m_numHandles;
	}

	uint16_t getMaxHandles() const
	{
		return m_maxHandles;
	}

	uint16_t alloc()
	{
		if( m_numHandles < m_maxHandles )
		{
			uint16_t index = m_numHandles;
			++m_numHandles;

			uint16_t* dense = getDensePtr();
			uint16_t handle = dense[index];
			uint16_t* sparse = getSparsePtr();
			sparse[handle] = index;
			return handle;
		}

		return kInvalidHandle;
	}

	bool isValid( uint16_t aHandle ) const
	{
		uint16_t* dense = getDensePtr();
		uint16_t* sparse = getSparsePtr();
		uint16_t index = sparse[aHandle];

		return index < m_numHandles && dense[index] == aHandle;
	}

	void free( uint16_t aHandle )
	{
		uint16_t* dense = getDensePtr();
		uint16_t* sparse = getSparsePtr();
		uint16_t index = sparse[aHandle];
		--m_numHandles;
		uint16_t temp = dense[m_numHandles];
		dense[m_numHandles] = aHandle;
		sparse[temp] = index;
		dense[index] = temp;
	}

	void reset()
	{
		m_numHandles = 0;
		uint16_t* dense = getDensePtr();
		for( uint16_t i = 0, num = m_maxHandles; i < num; ++i )
		{
			dense[i] = i;
		}
	}

private:
	HandleManager();

	uint16_t* getDensePtr() const
	{
		const uint8_t* ptr = reinterpret_cast<const uint8_t*>( this );
		return ( uint16_t* )&ptr[sizeof( HandleManager )];
	}

	uint16_t* getSparsePtr() const
	{
		return &getDensePtr()[m_maxHandles];
	}

	uint16_t m_numHandles;
	uint16_t m_maxHandles;
};

template <uint16_t MaxHandlesT>
class HandleManagerT : public HandleManager
{
public:
	HandleManagerT()
		: HandleManager( MaxHandlesT )
	{
	}

	~HandleManagerT()
	{
	}

private:
	uint16_t m_padding[2 * MaxHandlesT];
};