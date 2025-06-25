# Hippo Shell
Leveraging the capabilities of ollama, <b>hippo-shell</b> is a
linux assistant that can automatically execute shell commands offering a
shell-less experience where developers, system administrators and users can
accelerate their bash, system tasks.

### Example Case

The agent can automatically (either with or without user supervision) execute shell commands like the following

User prompt

>I want to pull and execute a nginx container on port 8086


The agent will execute a code like this

<code>
docker pull nginx && 
docker run -t server -p 8086:8080 nginx
</code>

