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
    $source_config = $source->getConfigFor($this);
    $resource = clients_resource_load((int)$source_config['source']);
    $result = clients_call($resource);
    $value = array();
    $value['items'] = $result[0]->data;
    return new FeedsFetcherResult($value, 'text/xml');
  }

  /**
   * Source form.
   */
  public function sourceForm($source_config) {
    $form = array();
    $services = array();
    foreach(clients_resources_load() as $rid => $source) {
      $services[$rid] = $source['name'];
    }
    $form['source'] = array(
      '#type' => 'select',
      '#title' => t('Available clients'),
      '#default_value' => $source_config['source'],
      '#options' => $services,
      '#description' => t('Choose a client'),
    );

    return $form;
  }

}