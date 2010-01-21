<?php
// $Id$

/**
 * Class definition for Common Syndication Parser.
 * @author Django Beatty - adub
 *
 * Parses RSS and Atom feeds.
 */
class FeedsClientsParser extends FeedsParser {

  /**
   * Parses a raw string and returns a Feed object from it.
   */
  public function parse(FeedsFetcherResult $fetcherResult, FeedsSource $source) {
    // needs more work on content
    return new FeedsParserResult($fetcherResult->value);
    
  }

  /**
   * Build configuration form.
   */
  public function configForm(&$form_state) {
    $form = array();
    $services = array();
    foreach(clients_resources_load() as $rid => $source) {
      $services[$rid] = $source['name'];
    }
    $form['source'] = array(
      '#type' => 'select',
      '#title' => t('Available resources'),
      '#default_value' => $this->config['source'],
      '#options' => $services,
      '#description' => t('Choose a resource'),
    );
    return $form;
  }
  
  /**
   * Define default configuration.
   */
  public function configDefaults() {
    return array('source' => '');
  }

  /**
   * Return mapping sources.
   *
   * At a future point, we could expose data type information here,
   * storage systems like Data module could use this information to store
   * parsed data automatically in fields with a correct field type.
   */
  public function getMappingSources() {
    $resource = clients_resource_load((int)$this->config['source']);
    return clients_fields($resource);
  }

  /**
   * Source form.
   */
  public function sourceForm($source_config) {
    if($this->config['source']) {
      $form = array();
      
      // value doesn't work for some reason so use hidden
      $form['source'] =  array(
        '#type' => 'hidden',
        '#value' => $this->config['source'],
      );
      
      $resource = clients_resource_load((int)$this->config['source']);
      $source_config_info[] = array(t('Name'), $resource->name);
      foreach($resource->configuration['options'] as $field => $val) {
        if(is_array($val)){ 
          $val = implode(', ', $val);
        }
        $source_config_info[] = array(ucfirst($field), $val);
      }

      $form['info'] = array(
        '#value' => theme_table(array(t('Resource')), $source_config_info),
      );
   
      return $form;
    }
    else {
      return array();
    }

  }
}