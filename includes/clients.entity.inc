<?php

/**
 * @file
 * Provides classes for clients handler entities.
 */

/**
 * Base class for entities that function as handlers.
 *
 * Part of this could be abstracted -- see the controller class.
 */
class ClientsHandlerEntity extends Entity {

  /**
   * Constructor method.
   *
   * Overrides Entity::__construct().
   */
  function __construct(array $values = array(), $entityType = NULL) {
    // todo: refactor; use parent::()?
    $this->entityType = $entityType;
    $this->setUp();

    // Lump all data unto the object...
    foreach ($values as $field => $value) {
      $this->$field = $value;
    }

    // Connections defined in code are already unserialized.
    // TODO: check if still needed???
    if (isset($object->configuration) && !is_array($object->configuration)) {
      $this->configuration = unserialize($object->configuration);
    }

    return $this;
  }

}

/**
 * Base class for resource handlers.
 */
abstract class clients_resource_base extends ClientsHandlerEntity {

  /**
   * The name of the component the resource provides, eg block delta.
   */
  public $component;

  /**
   * The name of the connection the resource uses.
   */
  public $connection;

  /**
   * The connection handler the resource uses, lazy-loaded.
   */
  private $connection_handler;

  /**
   * Extra form elements specific to a class's edit form.
   *
   * This is the same pattern as node_form() -- just ignore the object behind
   * the curtain ;)
   *
   * This (so far) is common to all versions of Drupal Services.
   *
   * @param $form
   *  The main form array.
   * @param $form_state
   *  The form state from the main form, which you probably don't need anyway.
   *
   * @return
   *  A FormAPI form array. This will be merged in with basic data and the
   *  submit button added.
   *
   * @see clients_connection_form()
   * @see clients_connection_form()
   * @see clients_connection_form_submit()
   *
   * TODO: define an interface for the admin UI
   */
  function resourceSettingsFormAlter(&$form, &$form_state) {
  }

  /**
   * Get this resource's connection
   */
  function getConnection() {
    if (!isset($this->connection_handler)) {
      $this->connection_handler = entity_load_single('clients_connection', $this->connection);
    }
    return $this->connection_handler;
  }

}

/**
 * Common class for broken handlers.
 */
class ClientsHandlerEntityBroken extends ClientsHandlerEntity {

  function connectionSettingsFormAlter(&$form, &$form_state) {
    drupal_set_message(t('This handler is broken.'), 'warning');
  }

}
