#pragma once

#include <steemit/comment_history/comment_history_plugin.hpp>

#include <fc/api.hpp>

namespace steemit { namespace app {
   struct api_context;
} }

namespace steemit { namespace comment_history {

namespace detail
{
   class comment_history_api_impl;
}

struct comment_history_record
{
   string            author;
   string            permlink;
   uint32_t          block = 0;
   uint32_t          trx_in_block = 0;
   uint16_t          op_in_trx = 0;
};

class comment_history_api
{
   public:
      comment_history_api( const steemit::app::api_context& ctx );

      void on_api_startup();

      /**
      * @brief Gets comment history.
      * @param author The author.
      * @param permlink The permlink.
      * @returns Result.
      */
      vector<comment_history_record>   get_comment_history( string author, string permlink )const;

   private:
      std::shared_ptr< detail::comment_history_api_impl > my;
};

} } // steemit::comment_history

FC_REFLECT( steemit::comment_history::comment_history_record,
   (author)
   (permlink)
   (block)
   (trx_in_block)
   (op_in_trx)
)


FC_API( steemit::comment_history::comment_history_api,
   (get_comment_history)
)
