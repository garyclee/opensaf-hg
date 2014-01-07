immcfg -f proxy.xml
immcfg -f proxied.xml
amf-adm unlock-in safSu=1,safSg=2N,safApp=Proxy
amf-adm unlock safSu=1,safSg=2N,safApp=Proxy
amf-adm unlock-in safSu=1,safSg=2N,safApp=Proxied
amf-adm unlock safSu=1,safSg=2N,safApp=Proxied
echo "press any key to lock proxied and proxy SU"
read
amf-adm lock safSu=1,safSg=2N,safApp=Proxied
amf-adm lock-in safSu=1,safSg=2N,safApp=Proxied
amf-adm lock safSu=1,safSg=2N,safApp=Proxy
amf-adm lock-in safSu=1,safSg=2N,safApp=Proxy

