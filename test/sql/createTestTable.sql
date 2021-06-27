CREATE TABLE session(
	id int(10) unsigned not null auto_increment,
	session_id varchar(255) not null,
	username varchar(10) not null collate utf8_bin,
	user_lvl enum('standard', 'reseller', 'sysadmin', 'groupadm') not null default 'standard',
	ip_addr varchar(255) not null,
	port varchar(255) not null,
	created datetime not null,
	modified datetime not null,
	last_action varchar(255) null default null,

	constraint `PRIMARY` primary key(id),
	constraint session_id unique(session_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
