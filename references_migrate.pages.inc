<?php

function references_migrate_migrate_references_fields($form, &$form_state) {
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
  
  if (count($field_infos) > 0) {
    $field_options = array();
    
    foreach ($field_infos as $key => $field_info) {
      $translate_array = array('@field_name' => $field_info['field_name']);
      $instances = field_read_instances(
        array('field_name' => $field_info['field_name']),
        array('include_inactive' => TRUE, 'include_deleted' => TRUE)
      );
      $options_instances = array();
      foreach ($instances as $instance) {
        $options_instances[] = $instance['entity_type'] .':'. $instance['bundle'] .' ('. check_plain($instance['label']) .')';
      }
      $translate_array['@instances'] = implode(', ', $options_instances);
      $field_options[$key] = t('@field_name: Appears in [@instances]', $translate_array);
    }
    
    $form['field_information'] = array(
      '#type' => 'value',
      '#value' => $field_infos,
    );
    
    $form['field_options'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Which references fields do you wish to migrate?'),
      '#options' => $field_options,
      '#required' => TRUE,
    );
  
    return confirm_form($form, t('Are you sure you wish to migrate the selected references fields?'), '<front>');;
  }
  
  $form['message'] = array(
    '#markup' => t('There are no references fields that need to be migrated at this time.'),
  );
}

function references_migrate_migrate_references_fields_submit($form, &$form_state) {
  $values = $form_state['values'];
  $field_infos = $values['field_information'];
  
  // Define the batch.
  $batch = array(
    'operations' => array(),
    'finished' => '_references_migrate_batches_finished',
    'title' => t('Start migrating reference fields.'),
    'init_message' => t('Preparing to migrate reference fields.'),
    'progress_message' => t('Migrating references fields.'),
    'error_message' => t('Reference field could not migrate content successfully.'),
  );

  foreach ($values['field_options'] as $key => $field_option) {
    if (!empty($field_option)) {
      $batch['operations'][] = array(
        '_references_migrate_references_field', $key, $field_infos[$key],
      );
    }
  }
  batch_set($batch);
  batch_process();
}

function _references_migrate_batches_finished($success, $results, $operations) {
  drupal_set_message(t('Succesfully migrated requested reference fields!'));
}

function _references_migrate_references_field($field_key, $field_info, &$context) {
  $context['message'] = t("Processing @field_name", array('@field_name' => $field_info['field_name']));

  if (!in_array($field_info['type'], array('node_reference', 'user_reference'))) {
    drupal_set_message(t("Field @field is not a references field.", array('@field' => $field_name)), 'error');
  }
  else {
    _references_migrate_references_field_to_entityreference_field($field_key, $field_info, $context);
    drupal_set_message(t("Processed @field_name", array('@field_name' => $field_info['field_name'])));
  }
}
