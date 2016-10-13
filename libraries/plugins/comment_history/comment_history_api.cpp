#include <steemit/comment_history/comment_history_api.hpp>

namespace steemit { namespace comment_history {

namespace detail
{
   class comment_history_api_impl
   {
      public:
         comment_history_api_impl( steemit::app::application& app )
            :_app( app ) {}

         vector<comment_history_record>   get_comment_history( string author, string permlink )const;

         steemit::app::application& _app;
   };

   vector<comment_history_record>   comment_history_api_impl::get_comment_history( string author, string permlink )const
   {
      vector<comment_history_record> result;
      const auto& idx = _app.chain_database()->get_index_type< comment_history_index >().indices().get< by_permlink >();
      auto itr = idx.lower_bound( boost::make_tuple( author, permlink ) );

      while( itr != idx.end() && itr->author == author && itr->permlink == permlink )
      {
         comment_history_record record;
         record.author       = author;
         record.permlink     = permlink;
         record.block        = itr->block;
         record.trx_in_block = itr->trx_in_block;
         record.op_in_trx    = itr->op_in_trx;
         result.push_back(record);
         ++itr;
      }

      return result;
   }

} // detail

comment_history_api::comment_history_api( const steemit::app::api_context& ctx )
{
   my = std::make_shared< detail::comment_history_api_impl >( ctx.app );
}

void comment_history_api::on_api_startup() {}

vector<comment_history_record>   comment_history_api::get_comment_history( string author, string permlink )const
{
   return my->get_comment_history( author, permlink );
}

} } // steemit::comment_history
