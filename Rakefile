namespace :serial do
  desc "echo from serail"
  task :cat do
    sh "cat /dev/ttyACM0"
  end

  desc "log serial tty"
  task :log do
    log_number = Dir["log/log-*"].length + 1
    case RUBY_PLATFORM
    when 'x86_64-linux'
      sh "cat /dev/ttyACM0 > logs/log-#{log_number}"
    else
      puts "FIXME: OS not implemented"
    end
  end
end

desc "create tags"
task :ctags do

sh <<EOF
if [ -f tags ]
then
  rm tags
fi
EOF

  sh "ctags src/* lib/* $(find /usr/ -name Arduino.h 2>/dev/null | grep arduino/Arduino.h)"
end

desc "cleans up flies"
task :clean do
  sh "rm tags"
  sh "rm -r logs/*"
  sh "ino clean"
end
