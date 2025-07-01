#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <pwd.h>
#include <grp.h>

#define CONSUMER "Consumer"
#define INPUT_PIN 3  // 输入 IO 引脚编号
#define OUTPUT_PIN 0 // 输出 IO 引脚编号

/* Request a line as input with edge detection. */
static struct gpiod_line_request *request_input_line(const char *chip_path,
						     unsigned int offset,
						     const char *consumer)
{
	struct gpiod_request_config *req_cfg = NULL;
	struct gpiod_line_request *request = NULL;
	struct gpiod_line_settings *settings;
	struct gpiod_line_config *line_cfg;
	struct gpiod_chip *chip;
	int ret;

	chip = gpiod_chip_open(chip_path);
	if (!chip)
		return NULL;

	settings = gpiod_line_settings_new();
	if (!settings)
		goto close_chip;

	gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
	gpiod_line_settings_set_edge_detection(settings, GPIOD_LINE_EDGE_BOTH);
	/* Assume a button connecting the pin to ground, so pull it up... */
	gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_UP);
	/* ... and provide some debounce. */
	gpiod_line_settings_set_debounce_period_us(settings, 10000);

	line_cfg = gpiod_line_config_new();
	if (!line_cfg)
		goto free_settings;

	ret = gpiod_line_config_add_line_settings(line_cfg, &offset, 1,
						  settings);
	if (ret)
		goto free_line_config;

	if (consumer) {
		req_cfg = gpiod_request_config_new();
		if (!req_cfg)
			goto free_line_config;

		gpiod_request_config_set_consumer(req_cfg, consumer);
	}

	request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

	gpiod_request_config_free(req_cfg);

free_line_config:
	gpiod_line_config_free(line_cfg);

free_settings:
	gpiod_line_settings_free(settings);

close_chip:
	gpiod_chip_close(chip);

	return request;
}

static const char *edge_event_type_str(struct gpiod_edge_event *event)
{
	switch (gpiod_edge_event_get_event_type(event)) {
	case GPIOD_EDGE_EVENT_RISING_EDGE:
		return "Rising";
	case GPIOD_EDGE_EVENT_FALLING_EDGE:
		return "Falling";
	default:
		return "Unknown";
	}
}

static struct gpiod_line_request *
request_output_line(const char *chip_path, unsigned int offset,
		    enum gpiod_line_value value, const char *consumer)
{
	struct gpiod_request_config *req_cfg = NULL;
	struct gpiod_line_request *request = NULL;
	struct gpiod_line_settings *settings;
	struct gpiod_line_config *line_cfg;
	struct gpiod_chip *chip;
	int ret;

	chip = gpiod_chip_open(chip_path);
	if (!chip)
		return NULL;

	settings = gpiod_line_settings_new();
	if (!settings)
		goto close_chip;

	gpiod_line_settings_set_direction(settings,
					  GPIOD_LINE_DIRECTION_OUTPUT);
	gpiod_line_settings_set_output_value(settings, value);

	line_cfg = gpiod_line_config_new();
	if (!line_cfg)
		goto free_settings;

	ret = gpiod_line_config_add_line_settings(line_cfg, &offset, 1,
						  settings);
	if (ret)
		goto free_line_config;

	if (consumer) {
		req_cfg = gpiod_request_config_new();
		if (!req_cfg)
			goto free_line_config;

		gpiod_request_config_set_consumer(req_cfg, consumer);
	}

	request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

	gpiod_request_config_free(req_cfg);

free_line_config:
	gpiod_line_config_free(line_cfg);

free_settings:
	gpiod_line_settings_free(settings);

close_chip:
	gpiod_chip_close(chip);

	return request;
}

static const char * value_str(enum gpiod_line_value value)
{
	if (value == GPIOD_LINE_VALUE_ACTIVE)
		return "Active";
	else if (value == GPIOD_LINE_VALUE_INACTIVE) {
		return "Inactive";
	} else {
		return "Unknown";
	}
}

static enum gpiod_line_value toggle_line_value(enum gpiod_line_value value)
{
	return (value == GPIOD_LINE_VALUE_ACTIVE) ? GPIOD_LINE_VALUE_INACTIVE :
						    GPIOD_LINE_VALUE_ACTIVE;
}

void daemonize() {
    pid_t pid;

    // 创建子进程
    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    // 退出父进程
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // 创建新的会话
    if (setsid() < 0) {
        perror("Setsid failed");
        exit(EXIT_FAILURE);
    }

    // 改变工作目录
    if (chdir("/") < 0) {
        perror("Chdir failed");
        exit(EXIT_FAILURE);
    }

    // 重设文件权限掩码
    umask(0);

    // 关闭标准文件描述符
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

static int codecinit(void)
{
    // 初始化Codec Mixer
    system("amixer -c ES8388 cset name='PCM Volume' 192");
    system("amixer -c ES8388 cset name='Headphone Playback Volume' 32");
    system("amixer -c ES8388 cset name='Speaker Playback Volume' 32");
    system("amixer -c ES8388 cset name='Left Mixer Left Playback Switch' on");
    system("amixer -c ES8388 cset name='Right Mixer Right Playback Switch' on");
    system("amixer -c ES8388 cset name='Differential Mux' 0");
    system("amixer -c ES8388 cset name='Left PGA Mux' 0");
    system("amixer -c ES8388 cset name='Right PGA Mux' 0");
    system("amixer -c ES8388 cset name='Left Channel Capture Volume' 8");
    system("amixer -c ES8388 cset name='Right Channel Capture Volume' 8");
    system("amixer -c ES8388 cset name='Mono Mux' 2");

    return 0;
}

void check_and_fix_ownership(const char *path, const char *target_user, const char *target_group) {

    struct stat file_stat;
    if (stat(path, &file_stat) != 0) {
        syslog(LOG_ERR, "Failed to get file status for %s: %s", path, strerror(errno));
        closelog();
        return;
    }

    struct passwd *user = getpwuid(file_stat.st_uid);
    struct group  *group = getgrgid(file_stat.st_gid);

    if (!user || !group) {
        syslog(LOG_ERR, "Failed to retrieve user or group info for %s", path);
        closelog();
        return;
    }

    //syslog(LOG_INFO, "Current owner of %s: %s", path, user->pw_name);
    //syslog(LOG_INFO, "Current group of %s: %s", path, group->gr_name);

    if (strcmp(user->pw_name, target_user) != 0 || strcmp(group->gr_name, target_group) != 0) {
        struct passwd *target_pw = getpwnam(target_user);
        struct group  *target_gr = getgrnam(target_group);

        if (!target_pw || !target_gr) {
            syslog(LOG_ERR, "Target user or group %s not found", target_user);
            closelog();
            return;
        }

        if (chown(path, target_pw->pw_uid, target_gr->gr_gid) != 0) {
            syslog(LOG_ERR, "Failed to change owner/group of %s: %s", path, strerror(errno));
            closelog();
            return;
        }

        syslog(LOG_INFO, "Changed owner/group of %s to %s", path, target_user);
    }
}

int main(void) {
    openlog("hpdet", LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "hpdet Starting");

    // 守护进程化
    daemonize();  

    codecinit();
    syslog(LOG_INFO, "Codec Inited");

    const char *path = "/run/user/1000/pulse";
    const char *target_user = "weston";
    const char *target_group = "weston";

    static const char *const chip_path = "/dev/gpiochip10";
    static const unsigned int hp_line_offset = INPUT_PIN;
    static const unsigned int spk_line_offset = OUTPUT_PIN;
    
    struct gpiod_edge_event_buffer *event_buffer;
    struct gpiod_line_request *in_request, *out_request;
    struct gpiod_edge_event *event;
    int i, ret, event_buf_size;
    
    enum gpiod_line_value value;
    
    in_request = request_input_line(chip_path, hp_line_offset,
                            "hp-inserted");
    if (!in_request) {
            fprintf(stderr, "failed to request line: %s\n",
                    strerror(errno));
            return EXIT_FAILURE;
    }

    /*
     * 获取HP的初始IO值，翻转一下，赋值给SPK
     */
    value = gpiod_line_request_get_value(in_request, hp_line_offset);
    out_request = request_output_line(chip_path, spk_line_offset, toggle_line_value(value),
				      "spk-en");
    if (!out_request) {
    	fprintf(stderr, "failed to request line: %s\n",
    		strerror(errno));
    	return EXIT_FAILURE;
    }

    // 等待pactl正常运行
    while(1) {
    	switch(value) {
    	    case GPIOD_LINE_VALUE_INACTIVE:
		// confirm the pulse permission is ok
		check_and_fix_ownership(path, target_user, target_group);
    	    	ret = system("su -l weston -c 'source /etc/profile.d/pulse_profile.sh; pactl set-sink-port @DEFAULT_SINK@ analog-output-speaker' ");
    	    	break;
    	    case GPIOD_LINE_VALUE_ACTIVE:
		// confirm the pulse permission is ok
		check_and_fix_ownership(path, target_user, target_group);
    	    	ret = system("su -l weston -c 'source /etc/profile.d/pulse_profile.sh; pactl set-sink-port @DEFAULT_SINK@ analog-output-headphones' ");
    	    	break;
    	    default:
    	    	break;
    	}

    	if (ret == 0) {
    	    break;
    	} else {
    	    sleep(1);
    	}
    }
    
    /*
    * 更大的缓冲区是对从内核中读取突发事件的优化，但在这种情况下不是必需的，所以l是可以的。
    */
    event_buf_size = 1;
    event_buffer = gpiod_edge_event_buffer_new(event_buf_size);
    if (!event_buffer) {
            fprintf(stderr, "failed to create event buffer: %s\n",
                    strerror(errno));
            return EXIT_FAILURE;
    }
    
    for (;;) {
            /* Blocks until at least one event is available. */
            ret = gpiod_line_request_read_edge_events(in_request, event_buffer,
                                                    event_buf_size);
            if (ret == -1) {
                    fprintf(stderr, "error reading edge events: %s\n",
                            strerror(errno));
                    return EXIT_FAILURE;
            }
            for (i = 0; i < ret; i++) {
                    event = gpiod_edge_event_buffer_get_event(event_buffer,
                                                            i);
                    syslog(LOG_INFO, "offset: %d  type: %-7s  event #%ld\n",
                    gpiod_edge_event_get_line_offset(event),
                    edge_event_type_str(event),
                    gpiod_edge_event_get_line_seqno(event));

	            switch (gpiod_edge_event_get_event_type(event)) {
        		case GPIOD_EDGE_EVENT_RISING_EDGE:
            			syslog(LOG_INFO, "HP Inserted, Inactive SPK\n");
				gpiod_line_request_set_value(out_request, spk_line_offset, GPIOD_LINE_VALUE_INACTIVE);
				// confirm the pulse permission is ok
				check_and_fix_ownership(path, target_user, target_group);
				system("su -l weston -c 'source /etc/profile.d/pulse_profile.sh; pactl set-sink-port @DEFAULT_SINK@ analog-output-headphones' ");
				break;
        		case GPIOD_EDGE_EVENT_FALLING_EDGE:
            			syslog(LOG_INFO, "HP Un-inserted, Active SPK\n");
				gpiod_line_request_set_value(out_request, spk_line_offset, GPIOD_LINE_VALUE_ACTIVE);
				// confirm the pulse permission is ok
				check_and_fix_ownership(path, target_user, target_group);
				system("su -l weston -c 'source /etc/profile.d/pulse_profile.sh; pactl set-sink-port @DEFAULT_SINK@ analog-output-speaker' ");
				break;
        		default:
            			syslog(LOG_INFO, "Unknown event\n");
				break;
        	    }
            }
    }
    
    gpiod_line_request_release(in_request);
    gpiod_line_request_release(out_request);

    closelog();

    return EXIT_SUCCESS;
}
