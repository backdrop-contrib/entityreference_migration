<?php

/**
 * Implementation of hook_drush_help().
 */
function references_migrate_drush_help($section) {
  switch ($section) {
    case 'drush:entityreference-migrate-references':
      return dt('Used without parameters, this will convert all the references fields into entityreference fields.');
  }
}

/**
 * Implementation of hook_drush_command().
 */
function references_migrate_drush_command() {
  $items = array();
  $items['entityreference-migrate-references'] = array(
    'callback' => 'drush_entityreference_migrate_references',
    'description' => t('Convert references fields into entityreference fields.'),
    'arguments' => array(
      'type' => dt('Optional. Only convert the particular field.'),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'aliases' => array('emr'),
  );
  
  return $items;
}

/**
 * Convert the references fields to entityreference.
 */
function drush_entityreference_migrate_references($field_name = '') {
  $field_infos = array();
  // Read the singular field.
  if (!empty($field_name)) {
    $field_infos = field_read_fields(
      array('field_name' => $field_name),
      array('include_inactive' => TRUE, 'include_deleted' => TRUE)
    );
  }
  else {
    // Load all references fields.
    $node_field_infos = field_read_fields(
      array('type'=>'node_reference'),
      array('include_inactive' => TRUE, 'include_deleted' => TRUE)
    );
    $user_field_infos = field_read_fields(
      array('type'=>'user_reference'),
      array('include_inactive' => TRUE, 'include_deleted' => TRUE)
    );
    $field_infos = $node_field_infos + $user_field_infos;
  }
  
  $batch = array(
    'operations' => array(),
    'finished' => '_drush_references_migrate_batches_finished',
    'title' => t('Start migrating reference fields.'),
    'init_message' => dt('Preparing to migrate reference fields.'),
    'progress_message' => dt('Migrating references fields.'),
    'error_message' => dt('Reference field could not migrate content successfully.'),
  );
  
  foreach ($field_infos as $field_info) {
    $batch['operations'][] = array(
      '_drush_references_migrate_references_field', array($field_info),
    );
  }

  batch_set($batch);
  $batch =& batch_get();
  $batch['progressive'] = FALSE;
  drush_backend_batch_process();
}

function _drush_references_migrate_batches_finished($success, $results, $operations) {
  drush_print(dt('Succesfully migrated requested reference fields!'));
}

function _drush_references_migrate_references_field($field_info) {
  drush_print(dt("Processing @field_name", array('@field_name' => $field_info['field_name'])));

  drush_print_r($field_info);
  // Read the instances for copying to the entityreference variant.
  $instances = field_read_instances(
    array('field_name' => $field_info['field_name']),
    array('include_inactive' => TRUE, 'include_deleted' => TRUE)
  );
  drush_print_r($instances);

  if (!in_array($field_info['type'], array('node_reference', 'user_reference'))) {
    drush_set_error(dt("Field @field is not a references field.", array('@field' => $field_name)));
  }
  else {
//    _references_migrate_references_field_to_entityreference_field($field_info);
  }
}
