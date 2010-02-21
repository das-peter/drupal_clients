<?php
// $Id$

/**
 * @file
 * Home of the FeedsFileFetcher.
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
    
    // (feeds > alpha9)   
    $batch = new FeedsImportBatch();
    $batch->setItems($result[0]->data); 
    return $batch;
  }
}