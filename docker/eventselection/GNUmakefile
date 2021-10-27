# Put whatever name makes sense for the image here. No spaces or funny
# characters!
#NAME = heponhpc/hepnos
NAME = heponhpc/hepnosdemo

# Remember to increment VERSION as you make new versions!
VERSION=1.66

.PHONY: all build tag_latest release test shell

all: build

build:
	docker build --tag $(NAME):$(VERSION) --network host $(PWD)

tag_latest: build
	docker tag $(NAME):$(VERSION) $(NAME):latest

shell: 
	docker run --rm -it $(NAME):$(VERSION)

release: tag_latest
	@if ! docker images $(NAME) | awk '{ print $$2 }' | grep -q -F $(VERSION); then echo "$(NAME) version $(VERSION) is not yet built. Please run 'make build'"; false; fi
	docker push $(NAME):$(VERSION)
