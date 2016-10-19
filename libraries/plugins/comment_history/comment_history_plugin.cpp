#include <steemit/comment_history/comment_history_api.hpp>

#include <steemit/chain/comment_object.hpp>
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
      void post_operation( const operation_object& o );
      void on_applied_block( const signed_block& b );

      comment_history_plugin&       _self;

      bool _store_timestamp    = true;
      bool _store_his_content  = true;
      bool _store_last_content = true;
};

void comment_history_plugin_impl::pre_operation( const operation_object& o )
{
   auto& db = _self.database();

   string author;
   string permlink;
   comment_op_type op_type;
   bool create_entry = false;
   if( o.op.which() == operation::tag< comment_operation >::value )
   {
      comment_operation op = o.op.get< comment_operation >();
      author = op.author;
      permlink = op.permlink;
      op_type = comment_op_type::comment;
      create_entry = true;
   }
   else if( o.op.which() == operation::tag< delete_comment_operation >::value )
   {
      delete_comment_operation op = o.op.get< delete_comment_operation >();
      author = op.author;
      permlink = op.permlink;
      op_type = comment_op_type::delete_comment;
      create_entry = true;
   }

   if( create_entry )
   {
      #ifndef IS_LOW_MEM
         if( _store_his_content && !_store_last_content )
         {
            const auto ptr = db.find_comment( author, permlink );
            if( ptr != nullptr )
            {
               const auto& idx = db.get_index_type< comment_history_index >().indices().get< by_permlink >();
               auto itr = idx.lower_bound( boost::make_tuple( author, permlink ) );
               if( itr != idx.end() && itr->author == author && itr->permlink == permlink )
               {
                  db.modify( *itr, [&]( comment_history_object& cho )
                  {
                     cho.title         = ptr->title;
                     cho.body          = ptr->body;
                     cho.json_metadata = ptr->json_metadata;
                  });
               }
            }
         }
      #endif

      db.create< comment_history_object >( [&]( comment_history_object& cho )
      {
         cho.author       = author;
         cho.permlink     = permlink;
         cho.block        = o.block;
         cho.trx_in_block = o.trx_in_block;
         cho.op_in_trx    = o.op_in_trx;
         cho.op_type      = op_type;
      });
   }
}

void comment_history_plugin_impl::post_operation( const operation_object& o )
{
   #ifndef IS_LOW_MEM
      if( _store_last_content )
      {
         if( o.op.which() == operation::tag< comment_operation >::value )
         {
            auto& db = _self.database();

            //TODO optimization: perhaps no need to find, instead, modify the element at the end
            //     (the one should be just created on pre_operation(), indexed by_block)
            comment_operation op = o.op.get< comment_operation >();
            const auto comment_object = db.get_comment( op.author, op.permlink );
            const auto& idx = db.get_index_type< comment_history_index >().indices().get< by_permlink >();
            auto itr = idx.lower_bound( boost::make_tuple( op.author, op.permlink ) );
            if( itr != idx.end() && itr->author == op.author && itr->permlink == op.permlink )
            {
               db.modify( *itr, [&]( comment_history_object& cho )
               {
                  cho.title         = comment_object.title;
                  cho.body          = comment_object.body;
                  cho.json_metadata = comment_object.json_metadata;
               });
            }
         }
      }
   #endif
}

void comment_history_plugin_impl::on_applied_block( const signed_block& b )
{
   if( _store_timestamp )
   {
      auto& db = _self.database();

      //TODO optimization: perhaps no need to search, instead, walk reversely from the end
      const auto block_num = b.block_num();
      const auto& idx = db.get_index_type< comment_history_index >().indices().get< by_block >();
      auto itr = idx.lower_bound( block_num );
      while( itr != idx.end() && itr->block == block_num )
      {
         db.modify( *itr, [&]( comment_history_object& cho )
         {
            cho.time = b.timestamp;
         });
         ++itr;
      }
   }
}

} // detail

comment_history_plugin::comment_history_plugin( application* app )
   :plugin( app ), _my( new detail::comment_history_plugin_impl( *this ) ) {}

comment_history_plugin::~comment_history_plugin() {}

void comment_history_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg
)
{
   cli.add_options()
         ("comment-history-store-timestamp", boost::program_options::value<bool>()->default_value(true),
           "Whether or not store timestamp of each edit. Disable it to get slightly better write performance but worse query performance. (default is true)")
         ("comment-history-store-his-content", boost::program_options::value<bool>()->default_value(true),
           "Whether or not store full content BEFORE each edit. Require LOW_MEMORY_NODE be ON to enable. Enable it to get better performance when fetching historical content, but consume more memory. (default is true)")
         ("comment-history-store-last-content", boost::program_options::value<bool>()->default_value(true),
           "Whether or not store full content AFTER each edit. Require LOW_MEMORY_NODE be ON to enable. Enable it to get slightly better performance when fetching latest details of changes, but consume more memory. (default is true)")
         ;
   cfg.add(cli);
}

void comment_history_plugin::plugin_initialize( const boost::program_options::variables_map& options )
{
   try
   {
      ilog( "comment_history_plugin: plugin_initialize() begin" );

      database().pre_apply_operation.connect( [&]( const operation_object& o ){ _my->pre_operation( o ); } );
      database().post_apply_operation.connect( [&]( const operation_object& o ){ _my->post_operation( o ); } );
      database().applied_block.connect( [&]( const signed_block& b ){ _my->on_applied_block( b ); } );

      database().add_index< primary_index< comment_history_index > >();

      if( options.count( "comment-history-store-timestamp" ) )
         _my->_store_timestamp = options[ "comment-history-store-timestamp" ].as< bool >();
      if( options.count( "comment-history-store-his-content" ) )
         _my->_store_his_content = options[ "comment-history-store-his-content" ].as< bool >();
      if( options.count( "comment-history-store-last-content" ) )
         _my->_store_last_content = options[ "comment-history-store-last-content" ].as< bool >();

      wlog( "comment-history-store-timestamp: ${b}", ("b", is_store_timestamp()) );
      wlog( "comment-history-store-his-content: ${b}", ("b", is_store_his_content()) );
      wlog( "comment-history-store-last-content: ${b}", ("b", is_store_last_content()) );

      ilog( "comment_history_plugin: plugin_initialize() end" );
   } FC_CAPTURE_AND_RETHROW()
}

void comment_history_plugin::plugin_startup()
{
   ilog( "comment_history plugin: plugin_startup() begin" );

   app().register_api_factory< comment_history_api >( "comment_history_api" );

   ilog( "comment_history plugin: plugin_startup() end" );
}

bool comment_history_plugin::is_store_timestamp() const
{
   return _my->_store_timestamp;
}

bool comment_history_plugin::is_store_his_content() const
{
   return _my->_store_his_content;
}

bool comment_history_plugin::is_store_last_content() const
{
   return _my->_store_last_content;
}


} } // steemit::comment_history

STEEMIT_DEFINE_PLUGIN( comment_history, steemit::comment_history::comment_history_plugin );
