desc "Build and upload to arduino"
task :release do
  sh "ino build && ino upload"
end

namespace :serial do
  desc "echo from serail"
  task :cat do
    sh "ino serial -b 115200"
  end

  desc "log serial tty"
  task :log do
    log_number = Dir["log/log-*"].length + 1
    case RUBY_PLATFORM
    when 'x86_64-linux'
      sh "cat /dev/ttyACM0 > logs/log-#{log_number}"
    else
      puts "FIXME: your OS not implemented"
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

  sh "ctags src/* lib/SdFat/* $(find /usr/ -name Arduino.h 2>/dev/null | grep arduino/Arduino.h)"
end

namespace :vendor do
  desc "Builds Utilities"
  task :build do
    sh "rm -f bin/bintocsv"
    sh "cc vendor/bintocsv.cpp -o bin/bintocsv -O2"
  end

end

desc "cleans up flies"
task :clean do
  sh "rm tags"
  sh "rm -r logs/*"
  sh "ino clean"
end
