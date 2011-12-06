#ifdef WINNT
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "address.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "environment.h"

#ifdef WINNT
#define sprintf sprintf_s
#endif

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Address::Address( int Port):
      port( Port),
      family( Empty)
{
   // There is no need to find out local host address.
   // Server will look at connected client address to register.
   memset( addr, 0, AddrDataLength);
}

Address::Address( const Address & other)
{
   copy( other);
}

Address::Address( const Address * other)
{
   if( other ) copy( *other);
}

Address & Address::operator=( const Address & other)
{
   if( this != &other) copy( other);
   return *this;
}

void Address::copy( const Address & other)
{
   port = other.port;
   family = other.family;
   memcpy( &addr, &other.addr, AddrDataLength);
}

Address::Address( const struct sockaddr_storage & ss)
{
   memset( addr, 0, AddrDataLength);
   switch( ss.ss_family)
   {
   case AF_INET:
   {
      struct sockaddr_in * sa = (struct sockaddr_in*)(&ss);
      family = IPv4;
      port = ntohs( sa->sin_port);
      memcpy( addr, &(sa->sin_addr), 4);
      break;
   }
   case AF_INET6:
   {
      struct sockaddr_in6 * sa = (struct sockaddr_in6*)(&ss);

      // Check for mapped IPv4 address: 1-80 bits == 0, 81-96 bits == 1 and last 32 bits == IPv4
      family = IPv4;
      char * data = (char*)&(sa->sin6_addr);
      for( int i = 0; i < 10; i++) // Check first 10 bytes for zeros:
         if( data[i] != char(0))
         {
            family = IPv6;
            break;
         }
      if( family == IPv4 )
         for( int i = 10; i < 12; i++) // Check next 2 bytes for 0xFF:
            if( data[i] != char(0xFF))
            {
               family = IPv6;
               break;
            }

      if( family == IPv6)
      {
         port = ntohs( sa->sin6_port);
         memcpy( addr, &(sa->sin6_addr), 16);
      }
      else
      {
         struct sockaddr_in * sa = (struct sockaddr_in*)(&ss);
         port = ntohs( sa->sin_port);
         memcpy( addr, &(data[12]), 4);
      }
      break;
   }
   default:
      AFERROR("Address::Address: Unknown address family.");
      break;
   }
}

bool Address::equal( const af::Address & other ) const
{
   if( isEmpty()) return false;
   if( other.isEmpty()) return false;

   if(( family == other.family) && ( port == other.port ))
   {
      if( memcmp( &addr, &(other.addr), AddrDataLength) == 0)
      return true;
   }
   return false;
}

Address::Address( const std::string & str)
{
   clear();
   if( str.empty()) return;

   unsigned int ip6[8];
   int fields6 = sscanf( str.c_str(), "%x:%x:%x:%x:%x:%x:%x:%x", &(ip6[0]),&(ip6[1]),&(ip6[2]),&(ip6[3]),&(ip6[4]),&(ip6[5]),&(ip6[6]),&(ip6[7]));
   if( fields6 == 8 )
   {
      family = IPv6;
      for( int i = 0; i < 8; i++)
      {
         uint8_t byte0 = ip6[i] >> 8;
         uint8_t byte1 = ip6[i] - (byte0 << 8);
         addr[2*i+0]   = byte0;
         addr[2*i+1]   = byte1;
      }
      return;
   }

   unsigned int ip4[4];
   int fields4 = sscanf( str.c_str(), "%u.%u.%u.%u", &(ip4[0]),&(ip4[1]),&(ip4[2]),&(ip4[3]));
   if( fields4 == 4 )
   {
      family = IPv4;
      for( int i = 0; i < 4; i++) addr[i] = ip4[i];
      return;
   }
   AFERRAR("String \"%s\" is not an IP address.", str.c_str())
}

Address::Address( Msg * msg)
{
   read( msg);
}

void Address::readwrite( Msg * msg)
{
   rw_int8_t(   family, msg);
   rw_uint16_t( port,   msg);
   rw_data(     addr,   msg, AddrDataLength);
}

Address::~Address()
{
}

void Address::clear()
{
   port = 0,
   family = Empty;
   memset( addr, 0, AddrDataLength);
}

#ifndef WINNT
bool Address::setSocketAddress( struct sockaddr_storage & ss) const
{
   bzero( &ss, sizeof(sockaddr_storage));
   switch( family)
   {
      case IPv4:
      {
         struct sockaddr_in * sa = (struct sockaddr_in*)(&ss);
         sa->sin_family = AF_INET;
         sa->sin_port = htons( port);
         memcpy( &(sa->sin_addr), addr, 4);
         break;
      }
      case IPv6:
      {
         struct sockaddr_in6 * sa = (struct sockaddr_in6*)(&ss);
         sa->sin6_family = AF_INET6;
         sa->sin6_port = htons( port);
         memcpy( &(sa->sin6_addr), addr, 16);
         break;
      }
      case Empty:
         AFERROR("Address::setSocketAddress: Address is empty.");
         return false;
      default:
         AFERROR("Address::setSocketAddress: Unknown address family.");
         return false;
   }
   return true;
}
#endif

void Address::setIP( const af::Address & other)
{
   family = other.family;
   memcpy( &addr, &other.addr, AddrDataLength);
}

void Address::generateIPStream( std::ostringstream & stream, bool full) const
{
   switch( family)
   {
      case IPv4:
      {
         char buffer[64];
         sprintf( buffer, "%u.%u.%u.%u", uint8_t(addr[0]), uint8_t(addr[1]), uint8_t(addr[2]), uint8_t(addr[3]));
         stream << buffer;
         break;
      }
      case IPv6:
      {
         char buffer[64];
         for( int i = 0; i < 8; i++)
         {
            if( i != 0 ) stream << ':';
            uint8_t byte0 = uint8_t(addr[2*i]);
            uint8_t byte1 = uint8_t(addr[2*i+1]);
            if( full )
            {
               if( byte0 ) sprintf( buffer, "%x%02x", byte0, byte1);
               else if( byte1 ) sprintf( buffer, "%x", byte1);
               if( byte0 || byte1 ) stream << buffer;
            }
            else
            {
               sprintf( buffer, "%02x%02x", byte0, byte1);
               stream << buffer;
            }
         }
         break;
      }
      case Empty:
         if( full ) stream << "Empty address";
         break;
      default:
         if( full ) stream << "Unknown address family";
         else stream << "AF=" << family;
         break;
   }
}

const std::string Address::generateIPString( bool full) const
{
   std::ostringstream stream;
   generateIPStream( stream, full);
   return stream.str();
}

void Address::generatePortStream( std::ostringstream & stream, bool full) const
{
   stream << port;
}

const std::string Address::generatePortString( bool full) const
{
   std::ostringstream stream;
   generatePortStream( stream, full);
   return stream.str();
}

void Address::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full )
   {
      switch (family)
      {
         case IPv4:
            stream << "IPv4 Address: ";
            break;
         case IPv6:
            stream << "IPv6 Address: ";
            break;
      }
   }
   generateIPStream( stream, true);
   if( notEmpty() && ( port != 0))
   {
      stream << ":";
      generatePortStream( stream, full);
   }
}

int Address::calcWeight() const
{
   int weight = sizeof( Address);
   return weight;
}