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
   time_point_sec    time;
   comment_op_type   op_type;
   string            title;
   string            body;
   string            json_metadata;
};

class comment_history_api
{
   public:
      comment_history_api( const steemit::app::api_context& ctx );

      void on_api_startup();

      /**
      * @brief Gets comment history. Newest first.
      * @param author The author.
      * @param permlink The permlink.
      * @param oldest No older than this time point.
      * @param newest No newer than this time point.
      * @param limit No more than this number of entries.
      * @param with_content Whether return contents.
      * @returns Result.
      */
      vector<comment_history_record>   get_comment_history( string author, string permlink, time_point_sec oldest, time_point_sec newest, uint32_t limit, bool with_content )const;

      /**
      * @brief Gets historical comment content at a given point.
      * @param author The author.
      * @param permlink The permlink.
      * @param block The block number.
      * @param trx_in_block The transaction sequence id in the given block.
      * @param op_in_trx The operation sequence id in the transaction.
      * @returns Result.
      */
      optional<comment_history_record> get_comment_history_record( string author, string permlink, uint32_t block, uint32_t trx_in_block, uint16_t op_in_trx )const;

      /**
      * @brief Gets historical comment content at a given time point.
      * @param author The author.
      * @param permlink The permlink.
      * @param time The time point.
      * @returns Result.
      */
      optional<comment_history_record> get_comment_at_time( string author, string permlink, time_point_sec time )const;

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
   (time)
   (op_type)
   (title)
   (body)
   (json_metadata)
)


FC_API( steemit::comment_history::comment_history_api,
   (get_comment_history)
   (get_comment_history_record)
   (get_comment_at_time)
)
