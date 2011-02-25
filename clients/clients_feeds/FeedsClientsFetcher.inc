<?php

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
    $items = isset($result[0]->data) ? $result[0]->data : array();
    
    // (feeds > alpha9)   
    $batch = new FeedsImportBatch();
    $batch->setItems($items); 
    return $batch;
  }
}