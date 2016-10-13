#include <steemit/comment_history/comment_history_api.hpp>

#include <steemit/chain/history_object.hpp>

#include <steemit/chain/database.hpp>

namespace steemit { namespace comment_history {

namespace detail
{

class comment_history_plugin_impl
{
   public:
      comment_history_plugin_impl( comment_history_plugin& plugin )
         :_self( plugin ) {}
      virtual ~comment_history_plugin_impl() {}

      void pre_operation( const operation_object& o );

      comment_history_plugin&       _self;
};

void comment_history_plugin_impl::pre_operation( const operation_object& o )
{
   auto& db = _self.database();

   if( o.op.which() == operation::tag< comment_operation >::value )
   {
      comment_operation op = o.op.get< comment_operation >();
      db.create< comment_history_object >( [&]( comment_history_object& cho )
      {
         cho.author       = op.author;
         cho.permlink     = op.permlink;
         cho.block        = o.block;
         cho.trx_in_block = o.trx_in_block;
         cho.op_in_trx    = o.op_in_trx;
      });
   }
   else if( o.op.which() == operation::tag< delete_comment_operation >::value )
   {
      delete_comment_operation op = o.op.get< delete_comment_operation >();
      db.create< comment_history_object >( [&]( comment_history_object& cho )
      {
         cho.author       = op.author;
         cho.permlink     = op.permlink;
         cho.block        = o.block;
         cho.trx_in_block = o.trx_in_block;
         cho.op_in_trx    = o.op_in_trx;
      });
   }
}

} // detail

comment_history_plugin::comment_history_plugin( application* app )
   :plugin( app ), _my( new detail::comment_history_plugin_impl( *this ) ) {}

comment_history_plugin::~comment_history_plugin() {}

void comment_history_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg
) {}

void comment_history_plugin::plugin_initialize( const boost::program_options::variables_map& options )
{
   try
   {
      ilog( "comment_history_plugin: plugin_initialize() begin" );

      database().pre_apply_operation.connect( [&]( const operation_object& o ){ _my->pre_operation( o ); } );

      database().add_index< primary_index< comment_history_index > >();

      ilog( "comment_history_plugin: plugin_initialize() end" );
   } FC_CAPTURE_AND_RETHROW()
}

void comment_history_plugin::plugin_startup()
{
   ilog( "comment_history plugin: plugin_startup() begin" );

   app().register_api_factory< comment_history_api >( "comment_history_api" );

   ilog( "comment_history plugin: plugin_startup() end" );
}

} } // steemit::comment_history

STEEMIT_DEFINE_PLUGIN( comment_history, steemit::comment_history::comment_history_plugin );
