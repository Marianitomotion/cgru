#pragma once

#include "client.h"

namespace af
{
/// Monitor - Afanasy GUI.
class Monitor: public Client
{
public:

/// Construct client, getting values from environment.
   Monitor();

/// Construct Monitor from buffer.
   Monitor( Msg * msg);

	/// Construct Monitor from JSON:
	Monitor( const JSON & obj);

   virtual ~Monitor();

	inline bool isListeningPort() const { return m_listening_port; }

   bool hasEvent( int type) const;

   inline long long getTimeActivity()      const { return m_time_activity;     }
   inline size_t    getJobsUsersIdsCount() const { return jobsUsersIds.size(); }
   inline size_t    getJobsIdsCount()      const { return jobsIds.size();      }

   inline const std::list<int32_t> * getJobsUsersIds() const { return &jobsUsersIds; }
   inline const std::list<int32_t> * getJobsIds()      const { return &jobsIds;      }

   void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

   static const int EventsCount;
   static const int EventsShift;

protected:
   bool  *  events;
   std::list<int32_t> jobsUsersIds;
   std::list<int32_t> jobsIds;
   int64_t m_time_activity;     ///< Last activity

private:
   bool construct();
   void readwrite( Msg * msg); ///< Read or write Monitor in buffer.

	bool m_listening_port;
};
}
