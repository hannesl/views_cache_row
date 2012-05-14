<?php

/**
 * Implementation of hook_views_plugins().
 */
function views_rowcache_views_plugins() {
  return array(
    'row' => array(
      'views_rowcache_node' => array(
        'title' => t('Content, cached'),
        'help' => t('Display the content with standard node view, cached by row.'),
        'handler' => 'views_rowcache_plugin_node_view',
        'base' => array('node'), // only works with 'node' as base.
        'uses options' => TRUE,
        'type' => 'normal',
        'help topic' => 'style-node',
      ),
    ),
  );
}
