#include <steemit/comment_history/comment_history_api.hpp>

namespace steemit { namespace comment_history {

namespace detail
{
   class comment_history_api_impl
   {
      public:
         comment_history_api_impl( steemit::app::application& app )
            :_app( app ) {}

         vector<comment_history_record>   get_comment_history( string author,
                                                               string permlink,
                                                               time_point_sec oldest,
                                                               time_point_sec newest,
                                                               uint32_t limit,
                                                               bool with_content )const;

         optional<comment_history_record> get_comment_history_record( string author,
                                                                      string permlink,
                                                                      uint32_t block,
                                                                      uint32_t trx_in_block,
                                                                      uint16_t op_in_trx )const;

         optional<comment_history_record> get_comment_at_time( string author,
                                                               string permlink,
                                                               time_point_sec time )const;

         steemit::app::application& _app;
   };

   vector<comment_history_record>   comment_history_api_impl::get_comment_history( string author,
                                                               string permlink,
                                                               time_point_sec oldest,
                                                               time_point_sec newest,
                                                               uint32_t limit,
                                                               bool with_content )const
   {
      //TODO following code works only for full nodes with all options enabled, need implementation for others
      vector<comment_history_record> result;
      const auto& idx = _app.chain_database()->get_index_type< comment_history_index >().indices().get< by_permlink_time >();
      auto itr = idx.lower_bound( boost::make_tuple( author, permlink, newest ) );

      while( itr != idx.end() && itr->author == author && itr->permlink == permlink
            && itr->time >= oldest && result.size() < limit )
      {
         comment_history_record record;
         record.author       = author;
         record.permlink     = permlink;
         record.block        = itr->block;
         record.trx_in_block = itr->trx_in_block;
         record.op_in_trx    = itr->op_in_trx;
         record.time         = itr->time;
         record.op_type      = itr->op_type;
         if( with_content )
         {
            record.title         = itr->title;
            record.body          = itr->body;
            record.json_metadata = itr->json_metadata;
         }
         result.push_back(record);
         ++itr;
      }

      return result;
   }

   optional<comment_history_record> comment_history_api_impl::get_comment_history_record( string author,
                                                                  string permlink,
                                                                  uint32_t block,
                                                                  uint32_t trx_in_block,
                                                                  uint16_t op_in_trx )const
   {
      //TODO following code works only for full nodes with all options enabled, need implementation for others
      const auto& idx = _app.chain_database()->get_index_type< comment_history_index >().indices().get< by_permlink >();
      auto itr = idx.lower_bound( boost::make_tuple( author, permlink, block, trx_in_block, op_in_trx ) );
      if( itr != idx.end() && itr->author == author && itr->permlink == permlink
            && itr->block == block && itr->trx_in_block == trx_in_block
            && itr->op_in_trx == op_in_trx )
      {
         comment_history_record record;
         record.author        = author;
         record.permlink      = permlink;
         record.block         = itr->block;
         record.trx_in_block  = itr->trx_in_block;
         record.op_in_trx     = itr->op_in_trx;
         record.time          = itr->time;
         record.op_type       = itr->op_type;
         record.title         = itr->title;
         record.body          = itr->body;
         record.json_metadata = itr->json_metadata;
         return record;
      }
      return optional<comment_history_record>();
   }

   optional<comment_history_record> comment_history_api_impl::get_comment_at_time( string author,
                                                               string permlink,
                                                               time_point_sec time )const
   {
      //TODO following code works only for full nodes with all options enabled, need implementation for others
      const auto& idx = _app.chain_database()->get_index_type< comment_history_index >().indices().get< by_permlink_time >();
      auto itr = idx.lower_bound( boost::make_tuple( author, permlink, time ) );
      if( itr != idx.end() && itr->author == author && itr->permlink == permlink )
      {
         comment_history_record record;
         record.author        = author;
         record.permlink      = permlink;
         record.block         = itr->block;
         record.trx_in_block  = itr->trx_in_block;
         record.op_in_trx     = itr->op_in_trx;
         record.time          = itr->time;
         record.op_type       = itr->op_type;
         record.title         = itr->title;
         record.body          = itr->body;
         record.json_metadata = itr->json_metadata;
         return record;
      }
      return optional<comment_history_record>();
   }

} // detail

comment_history_api::comment_history_api( const steemit::app::api_context& ctx )
{
   my = std::make_shared< detail::comment_history_api_impl >( ctx.app );
}

void comment_history_api::on_api_startup() {}

vector<comment_history_record>   comment_history_api::get_comment_history( string author,
                                                               string permlink,
                                                               time_point_sec oldest,
                                                               time_point_sec newest,
                                                               uint32_t limit,
                                                               bool with_content )const
{
   return my->get_comment_history( author, permlink, oldest, newest, limit, with_content );
}

optional<comment_history_record> comment_history_api::get_comment_history_record( string author,
                                                                      string permlink,
                                                                      uint32_t block,
                                                                      uint32_t trx_in_block,
                                                                      uint16_t op_in_trx )const
{
   return my->get_comment_history_record( author, permlink, block, trx_in_block, op_in_trx );
}

optional<comment_history_record> comment_history_api::get_comment_at_time( string author,
                                                               string permlink,
                                                               time_point_sec time )const
{
   return my->get_comment_at_time( author, permlink, time );
}

} } // steemit::comment_history
