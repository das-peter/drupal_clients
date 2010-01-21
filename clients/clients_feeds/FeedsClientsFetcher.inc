<?php
// $Id$

/**
 * @file
 * @author Django Beatty - adub
 */

/**
 * Fetches data via HTTP.
 */
class FeedsClientsFetcher extends FeedsFetcher {

  /**
   * Fetch a local resource.
   */
  public function fetch(FeedsSource $source) {
    $source_config = $source->getConfig();
    $resource = clients_resource_load((int)$source_config['FeedsClientsParser']['source']);
    $result = clients_call($resource);
    $value = array();
    $value['items'] = $result[0]->data;
    return new FeedsFetcherResult($value, 'text/xml');
  }
}