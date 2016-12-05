SUBDIRS = lib Server Client Control

all:
	@for n in $(SUBDIRS); do make -C $$n; done

clean:
	@for n in $(SUBDIRS); do make clean -C $$n; done

