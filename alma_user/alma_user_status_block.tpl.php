<?php
// $Id$

/**
 * @file alma_user_status_block.tpl.php
 * Template for the user status block.
 */
?>
<div class="logout-link"><?php print l('Log out', 'logout'); ?></div>

<div class="welcome-username">
  <span class="welcome"><?php print t('Welcome'); ?></span>
  <span class="user-name"><?php print l($display_name, 'user/' . $user->uid . '/profile'); ?></span>
</div>

<div class="alma-patron-status <?php print $wrapper_classes; ?>">
<?php if (!$user_status_unavailable): ?>
  <div class="cart-status">
    <span class="count"><?php print $cart_count; ?></span>
    <?php print l('Go to cart', 'patron/cart'); ?>
  </div>
  <ul class="materials">
    <li class="loans">
      <?php print l(t("Loans") . ' <span class="count">' . $user_status['loan_count'] . '</span>', 'user/'. $user->uid . '/status', array('html' => TRUE)); ?>
    </li>
    <li class="reservations">
      <?php print l(t("Reservations") . ' <span class="count">' . $user_status['reservation_count'] . '</span>', 'user/'. $user->uid . '/status', array('html' => TRUE, 'fragment' => 'reservation')); ?>
    </li>
  </ul>
<?php else: ?>
  <div class="status-unavailable">
    <?php print $status_unavailable_message; ?>
  </div>
<?php endif; ?>
</div>

