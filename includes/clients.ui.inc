<?php

/**
 * @file
 * Provides a controller for building an entity overview form.
 */

/**
 * Default controller for providing handler UI.
 */
class ClientsHandlerEntityUIController extends EntityDefaultUIController {

  /**
   * Provides definitions for implementing hook_menu().
   */
  public function hook_menu() {
    // Start with what EntityAPI gives us, then doctor it.
    $items = parent::hook_menu();

    // Change the EntityAPI to deal with multiple handler types.
    $items[$this->path . '/add']['title'] = 'Add TODO';
    $items[$this->path . '/add']['page callback'] = 'clients_handler_add_page';
    $items[$this->path . '/add']['page arguments'] = array($this->entityType);
    //$items[$this->path . '/add']['file'] = 'fragment.admin.inc'; // TODO
    // Need to specify file path as this gets used in entity_menu().
    //$items[$this->path . '/add']['file path'] = drupal_get_path('module', $this->entityInfo['module']);
    // Extra fix; not in the patch yet.

    // Add links. Follow same pattern as node.
    $handler_types_callback = $this->entityInfo['admin ui']['types callback'];
    $handler_types = $handler_types_callback();
    foreach ($handler_types as $type => $type_info) {
      $items[$this->path . '/add/' . $type] = array(
        'title' => 'Create !name',
        'title arguments' => array('!name' => $type_info['label']),
        'page callback' => 'clients_handler_get_add_form',
        'page arguments' => array($this->entityType, $type),
        'file' =>  $this->entityInfo['admin ui']['file'],
        // Need to specify file path as this gets used in entity_menu().
        'file path' => drupal_get_path('module', $this->entityInfo['module']),
      );
    }

    // All menu items for handlers have the same access.
    // This means we don't need an access callback.
    foreach ($items as $path => $item) {
      $items[$path]['access callback'] = 'user_access';
      $items[$path]['access arguments'] = array($this->entityInfo['admin ui']['access permission']);
    }

    return $items;
  }

}

/**
 * UI controller class for connections.
 */
class ClientsConnectionEntityUIController extends ClientsHandlerEntityUIController {

  /**
   * Provides definitions for implementing hook_menu().
   */
  public function hook_menu() {
    // Tweak what our base class does..
    $items = parent::hook_menu();

    // Create the base item for the Clients admin tabs...
    $base_item = $items[$this->path];
    $base_item['title'] = t('Clients');
    $base_item['type'] = MENU_NORMAL_ITEM;
    $items['admin/structure/clients'] = $base_item;

    // ... and turn the connections base item into the first tab.
    $items[$this->path]['title'] = t('Connections');
    $items[$this->path]['type'] = MENU_DEFAULT_LOCAL_TASK;

    // TODO: testing system!!!

    return $items;
  }

  /**
   * Generates the table headers for the overview table.
   */
  protected function overviewTableHeaders($conditions, $rows, $additional_header = array()) {
    $additional_header['type'] = t('Type');
    $additional_header['endpoint'] = t('Endpoint');

    return parent::overviewTableHeaders($conditions, $rows, $additional_header);
  }

  /**
   * Generates the row for the passed entity and may be overridden in order to
   * customize the rows.
   *
   * @param $additional_cols
   *   Additional columns to be added after the entity label column.
   */
  protected function overviewTableRow($conditions, $id, $entity, $additional_cols = array()) {
    $additional_cols['type'] = $entity->type;
    $additional_cols['endpoint'] = $entity->formatEndpoint($entity->endpoint);

    return parent::overviewTableRow($conditions, $id, $entity, $additional_cols);
  }

}

/**
 * UI controller class for resources.
 */
class ClientsResourceEntityUIController extends ClientsHandlerEntityUIController {

  /**
   * Provides definitions for implementing hook_menu().
   */
  public function hook_menu() {
    // Tweak what our base class does..
    $items = parent::hook_menu();

    $items[$this->path]['title'] = t('Resources');
    $items[$this->path]['type'] = MENU_LOCAL_TASK;

    return $items;
  }

  /**
   * Generates the table headers for the overview table.
   */
  protected function overviewTableHeaders($conditions, $rows, $additional_header = array()) {
    $additional_header['type'] = t('Type');
    $additional_header['connection'] = t('Connection');

    return parent::overviewTableHeaders($conditions, $rows, $additional_header);
  }

  /**
   * Generates the row for the passed entity and may be overridden in order to
   * customize the rows.
   *
   * @param $additional_cols
   *   Additional columns to be added after the entity label column.
   */
  protected function overviewTableRow($conditions, $id, $entity, $additional_cols = array()) {
    $additional_cols['type'] = $entity->type;
    $additional_cols['connection'] = l($entity->connection, 'admin/structure/clients/connections/manage/' . $entity->connection);

    return parent::overviewTableRow($conditions, $id, $entity, $additional_cols);
  }

}
