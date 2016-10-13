#include <steemit/app/plugin.hpp>

#include <graphene/db/generic_index.hpp>

#include <boost/multi_index/composite_key.hpp>

//
// Plugins should #define their SPACE_ID's so plugins with
// conflicting SPACE_ID assignments can be compiled into the
// same binary (by simply re-assigning some of the conflicting #defined
// SPACE_ID's in a build script).
//
// Assignment of SPACE_ID's cannot be done at run-time because
// various template automagic depends on them being known at compile
// time.
//
#ifndef COMMENT_HISTORY_SPACE_ID
#define COMMENT_HISTORY_SPACE_ID 11
#endif

#ifndef COMMENT_HISTORY_PLUGIN_NAME
#define COMMENT_HISTORY_PLUGIN_NAME "comment_history"
#endif

namespace steemit { namespace comment_history {

using namespace chain;
using namespace graphene::db;
using app::application;

namespace detail
{
   class comment_history_plugin_impl;
}

class comment_history_plugin : public steemit::app::plugin
{
   public:
      comment_history_plugin( application* app );
      virtual ~comment_history_plugin();

      virtual std::string plugin_name()const override { return COMMENT_HISTORY_PLUGIN_NAME; }
      virtual void plugin_set_program_options(
         boost::program_options::options_description& cli,
         boost::program_options::options_description& cfg ) override;
      virtual void plugin_initialize( const boost::program_options::variables_map& options ) override;
      virtual void plugin_startup() override;

   private:
      friend class detail::comment_history_plugin_impl;
      std::unique_ptr< detail::comment_history_plugin_impl > _my;
};

struct comment_history_object : public abstract_object< comment_history_object >
{
   static const uint8_t space_id = COMMENT_HISTORY_SPACE_ID;
   static const uint8_t type_id = 0;

   string            author;
   string            permlink;
   uint32_t          block = 0;
   uint32_t          trx_in_block = 0;
   uint16_t          op_in_trx = 0;
};

struct by_permlink;
typedef multi_index_container<
   comment_history_object,
   indexed_by<
      hashed_unique< tag< by_id >, member< object, object_id_type, &object::id > >,
      ordered_unique< tag< by_permlink >,
         composite_key< comment_history_object,
            member< comment_history_object, string, &comment_history_object::author >,
            member< comment_history_object, string, &comment_history_object::permlink >,
            member< comment_history_object, uint32_t, &comment_history_object::block >,
            member< comment_history_object, uint32_t, &comment_history_object::trx_in_block >,
            member< comment_history_object, uint16_t, &comment_history_object::op_in_trx >
         >,
         composite_key_compare< std::less< string >,
                                std::less< string >,
                                std::less< uint32_t >,
                                std::less< uint32_t >,
                                std::less< uint16_t > >
      >
   >
> comment_history_object_multi_index_type;


typedef object_id< COMMENT_HISTORY_SPACE_ID, 0, comment_history_object >                  comment_history_object_id_type;

typedef generic_index< comment_history_object, comment_history_object_multi_index_type >  comment_history_index;

} } // steemit::comment_history

FC_REFLECT_DERIVED( steemit::comment_history::comment_history_object, (graphene::db::object),
   (author)
   (permlink)
   (block)
   (trx_in_block)
   (op_in_trx)
)
