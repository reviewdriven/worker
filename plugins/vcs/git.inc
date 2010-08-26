<?php
/**
 * @file
 * Provide basic git repository interface plugin.
 *
 * All git repository URLs should use the format demonstrated below:
 * cvs://user@example.com:git/stuff.git.
 *
 * @author Jimmy Berry ("boombatower", http://drupal.org/user/214218)
 */

$plugin = array(
  'title' => t('git'),
  'description' => t('Basic git repository interface plugin.'),
  'print' => 'worker_git_print',
  'checkout' => 'worker_git_checkout',
  'annotate' => 'worker_git_annotate',
  'revision' => 'worker_git_revision',
  'apply' => 'worker_patch_apply',
  'modified files' => 'worker_patch_modified_files',
);

/**
 * Get the short printable version of the URL.
 *
 * @param $url
 *   The repository URL.
 * @return
 *   Printable version of the URL.
 */
function worker_git_print($url) {
  return $url; // @TODO
}

/**
 * Checkout from the repository into working directory.
 *
 * @param $url
 *   The repository URL.
 * @return
 *   TRUE if successfull, otherwise FALSE.
 */
function worker_git_checkout($url) {
  $parts = explode('/', $url);
  $branch = escapeshellarg(end($parts));

  list(, $url) = explode('://', $url, 2);
  $url = substr($url, 0, -1 * strlen($branch) + 1);
  $url = escapeshellarg($url);

  return worker_execute("git clone -b $branch $url .");
}

/**
 * Annotate the file and return a map of line number to committer.
 *
 * @param $file
 *   Relative path to file.
 * @return
 *   Associative array of line number ot committer ID.
 */
function worker_git_annotate($file) {
  $lines = worker_execute('cvs annotate ' . escapeshellarg($file));

  $map = array();
  $number = 1;
  foreach ($lines as $line) {
    // Ignore empty lines and lines that do not start with a number or do not
    // match the regular expression. The annotation line format looks like the
    // following: 1.1         (some_username    10-Mar-01): <?php
    if (!empty($line) && is_numeric($line[0]) && preg_match('/.*?\s+\((.*?)\s+.*?\)|/', $line, $match)) {
      $map[$number++] = $match[1];
    }
  }
  return $map;
}

/**
 * Get the current revision of the code.
 *
 * @return
 *   Revision identifier.
 */
function worker_git_revision() {
  $lines = worker_execute_output('cvs log -bN');

  $latest = 0;
  foreach ($lines as $line) {
    if (preg_match('/date: (.*?);/', $line, $match)) {
      $latest = max(strtotime($match[1]), $latest);
    }
  }
  return $latest;
}