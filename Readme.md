# C on Azure appservice - Cat generator

Live demo : https://demo-cats-api-c.azurewebsites.net/

This repo is a proof of concept on using C on appservice, using containers.

The application itself just sends random cat images.

## Configuration

This app accepts two env variables

+ **API_KEY** : [Thecatapi.com](https://thecatapi.com) API Key. It's in fact not needed by the API endpoint used but the
  APP checks for it at boot.

## On the custom appservice runtime

To work as any other webapp, two requirements must be fulfilled :

+ There must be a process listening to port **80** to handle HTTP requests.
  (Although this is customizable via the **WEBSITES_PORT** env variable). An nginx reverse-proxy in this case.
+ An SSH server must be available, using port **2222**. As the Azure portal will login as root, the root password must
  be fixed to **Docker!**. To prevent this from being a security flaw, the app is running as a non-root user, and *
  should*
  have no way to switch user, even knowing the password.

Having these two requirements fulfilled, the remaining Dockerfile is focused on building the smallest possible image :

+ The builder step is cross-compiling the app to `x86_64-unknown-linux-musl`, as the target is alpine, which as no glibc
  support out of the box.
+ The compiled app is stripped and upx'ed
+ The release step is installing the runtime dependencies and copies the stripped and upx'ed app. The final image is
  17MB. 


