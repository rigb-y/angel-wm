.DEFAULT_GOAL = all

CC := gcc
CF := -std=c23 -MMD -MP \
	-Isrc \
	-Isrc/logging \
	-Isrc/error \
	-Isrc/types \
	-Isrc/setup \
	-Isrc/winscan \
	-Isrc/cleanup \
	-Isrc/client \
	-Isrc/client_list \
	-Isrc/utils \
	-Isrc/windows \
	-Isrc/manage \
	-Isrc/screen \
	-Isrc/layout/geometry \
	-Isrc/layout/motiontree \
	-Isrc/layout/motiontree/node \
	-Isrc/layout/motiontree/mtnl \
	-Isrc/events \
	-Isrc/keyboard \
	-Isrc/keyboard/keymaps \
	-Isrc/terminal \
	-Isrc/workspaces \
	-Isrc/colors \
	-Isrc/cursors \
	-Isrc/focus \
	-Isrc/pointer \
	-Isrc/float_list \
	-Isrc/detached \
	-Isrc/layout/layouts \
	-Isrc/shell \
	-Isrc/minimize \
	-Isrc/minimize/minimized_client \
	-Isrc/minimize/minimized_list \
	-Isrc/config \
	-Isrc/config/text_buffer \
	-Isrc/config/lexer \
	-Isrc/config/lexer/token \
	-Isrc/config/config_parser \
	-Isrc/config/config_error \
	-Isrc/config/strings \
	-Isrc/config/defaults \
	-Isrc/config/symbols \
	-Isrc/icccm \
	-Isrc/atoms \
	-Isrc/unmapped_client \
	-Isrc/unmapped_list \
	-Isrc/ewmh \
	-Isrc/monitors \
	-Isrc/monitor \
	-Isrc/randr \
	-Isrc/docks \
	-Isrc/wspipe \
	-Isrc/angel \

CF += $(EXTRA_CFLAGS)

SRC = src/main.c \
	  src/logging/logging.c \
	  src/error/error.c \
	  src/setup/setup.c \
	  src/winscan/winscan.c \
	  src/cleanup/cleanup.c \
	  src/client/client.c \
	  src/client_list/client_list.c \
	  src/utils/utils.c \
	  src/windows/windows.c \
	  src/manage/manage.c \
	  src/screen/screen.c \
	  src/layout/geometry/geometry.c \
	  src/layout/motiontree/motion_tree.c \
	  src/layout/motiontree/node/node.c \
	  src/layout/motiontree/mtnl/mtnl.c \
	  src/events/events.c \
	  src/keyboard/keyboard.c \
	  src/keyboard/keymaps/keymaps.c \
	  src/terminal/terminal.c \
	  src/workspaces/workspaces.c \
	  src/colors/colors.c \
	  src/cursors/cursors.c \
	  src/focus/focus_stack.c \
	  src/focus/focus.c \
	  src/pointer/pointer.c \
	  src/float_list/float_list.c \
	  src/detached/detached.c \
	  src/layout/layouts/layouts.c \
	  src/shell/shell.c \
	  src/minimize/minimized_client/minimized_client.c \
	  src/minimize/minimized_list/minimized_list.c \
	  src/config/text_buffer/text_buffer.c \
	  src/config/lexer/lexer.c \
	  src/config/lexer/token/token.c \
	  src/config/config_parser/config_parser.c \
	  src/config/config_parser/parser_utils.c \
	  src/config/config_error/config_error.c \
	  src/config/strings/angel_strings.c \
	  src/config/defaults/defaults.c \
	  src/config/symbols/symbol_table.c \
	  src/icccm/icccm.c \
	  src/atoms/atoms.c \
	  src/unmapped_client/unmapped_client.c \
	  src/unmapped_list/unmapped_list.c \
	  src/ewmh/ewmh.c \
	  src/monitors/monitors.c \
	  src/monitor/monitor.c \
	  src/randr/randr.c \
	  src/docks/docks.c \
	  src/wspipe/wspipe.c \
	  src/angel/angel.c \

LDFLAGS := 
LDFLAGS += $(EXTRA_LDFLAGS)

PROGRAM := angel

BUILD := release
BINDIR := bin/$(BUILD)
BUILDDIR := build/$(BUILD)
TARGET := $(BINDIR)/$(PROGRAM)

XSESSION_DIR ?= /usr/share/xsessions

OBJ := $(SRC:%.c=$(BUILDDIR)/%.o)

PREFIX ?= /usr/local
INSTALL_DIR ?= $(PREFIX)/bin
INSTALL ?= install

PKG_CONFIG ?= pkg-config
PKGS := x11 xrandr

CF += $(shell $(PKG_CONFIG) --cflags $(PKGS))
LDLIBS := $(shell $(PKG_CONFIG) --libs $(PKGS))

.PHONY: all clean install uninstall debug lsan asan check-deps

check-deps:
	@$(PKG_CONFIG) --exists $(PKGS) || { \
		echo "error: missing X11/Xrandr development libraries"; \
		exit 1; \
	}
	@command -v xwallpaper >/dev/null 2>&1 || { \
		echo "error: xwallpaper is required"; \
		exit 1; \
	}

debug:
	$(MAKE) BUILD=debug EXTRA_CFLAGS=-g all

lsan:
	$(MAKE) BUILD=lsan \
		EXTRA_CFLAGS="-fno-omit-frame-pointer -fsanitize=leak" \
		EXTRA_LDFLAGS="-fno-omit-frame-pointer -fsanitize=leak" \
		all
asan:
	$(MAKE) BUILD=asan \
		EXTRA_CFLAGS="-fno-omit-frame-pointer -fsanitize=address" \
		EXTRA_LDFLAGS="-fno-omit-frame-pointer -fsanitize=address" \
		all

all: check-deps $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(OBJ) $(LDLIBS) -o $@

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CF) -c $< -o $@

install: check-deps $(TARGET)
	$(INSTALL) -d "$(DESTDIR)$(INSTALL_DIR)"
	$(INSTALL) -d "$(DESTDIR)$(XSESSION_DIR)"
	$(INSTALL) -m 755 "$(TARGET)" "$(DESTDIR)$(INSTALL_DIR)/$(PROGRAM)"
	$(INSTALL) -m 755 "angel-log" "$(DESTDIR)$(INSTALL_DIR)/angel-log"
	$(INSTALL) -m 644 "sessions/angel.desktop" "$(DESTDIR)$(XSESSION_DIR)/angel.desktop"
	$(INSTALL) -m 644 "sessions/angel-log.desktop" "$(DESTDIR)$(XSESSION_DIR)/angel-log.desktop"

uninstall:
	$(RM) "$(DESTDIR)$(INSTALL_DIR)/$(PROGRAM)"
	$(RM) "$(DESTDIR)$(XSESSION_DIR)/angel.desktop"
	$(RM) "$(DESTDIR)$(XSESSION_DIR)/angel-log.desktop"

clean:
	$(RM) -r build bin

-include $(OBJ:.o=.d)
