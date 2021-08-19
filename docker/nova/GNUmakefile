# Put whatever name makes sense for the image here. No spaces or funny
# characters!
NAME = heponhpc/novas190703

# Remember to increment VERSION as you make new versions!
VERSION = 2.1

.PHONY: all build shell tag_latest release test

all: build

build:
	docker build --tag $(NAME):$(VERSION) $(PWD)

shell: build
	docker run --rm -it $(NAME):$(VERSION)

tag_latest: build
	docker tag $(NAME):$(VERSION) $(NAME):latest

release: tag_latest
	@if ! docker images $(NAME) | awk '{ print $$2 }' | grep -q -F $(VERSION); then echo "$(NAME) version $(VERSION) is not yet built. Please run 'make build'"; false; fi
	docker push $(NAME)
