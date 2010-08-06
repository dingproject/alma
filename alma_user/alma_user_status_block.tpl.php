<?php
// $Id$

/**
 * @file alma_user_status_block.tpl.php
 * Template for the user status block.
 */
 
/*
 * TODO get status from mikl
 */
if( $user_status['loan_overdue_count'] >= 1){
  $loan_status  = "warning";
}
else{
  $loan_status  = "default";
}
  
if( $user_status['reservation_count'] >= 1){
  $reservation_status = "ok";
}
else{
  $reservation_status = "default";
  
}
?>
<div id="account-profile" class="clearfix">
	<div class="user">

		<div class="logout">
			<?php print l(t('log out'), 'logout', array('attributes' => array('class' =>'logout'))); ?>
		</div>

		<h5><?php print t('Welcome'); ?></h5>
		<div class="username">
			<?php print l($display_name, $profile_link, array('attributes' => array('class' =>'username')));  ?>
		</div>

	</div>
	<?php if ($user_status_available): ?>
		<div class="cart">
	    <div class="count"><?php print $cart_count; ?></div>
	    <?php print l(t('Go to cart'), 'user/' . $user->uid . '/cart'); ?>
		</div>

		<ul>
	    <li>

	      <div class="content">
					<?php print l('<span>'.t("Loans") . '</span> <strong>' . $user_status['loan_count'] . '</strong>', 'user/'. $user->uid . '/status', array('html' => TRUE)); ?>
				</div>
        <?php if($loan_status != "default"){ ?>
				  <div class="status"><span class="<?php print $loan_status ?>">!</span></div>
        <?php } ?>
	    </li>
	    <li>
				<div class="content">
	        <?php print l('<span>'.t("Reservations") . '</span> <strong>' . $user_status['reservation_count'] . '</strong>', 'user/'. $user->uid . '/status', array('html' => TRUE, 'fragment' => 'reservation')); ?>
				</div>
        <?php if($reservation_status  != "default"){ ?>				
				  <div class="status"><span class="<?php print $reservation_status ?>">ok</span></div>
        <?php } ?>

	    </li>
	</ul>
	<?php else: ?>
	  <div class="status-unavailable">
	    <?php print $status_unavailable_message; ?>
	  </div>
	<?php endif; ?>
</div>
