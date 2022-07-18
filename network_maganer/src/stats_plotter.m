close all;

Array=readtable('teste1.csv');

%t = datetime(Array(:, 1),'InputFormat','yyyy.MM.dd HH:mm:ss.SSS');
t = datetime(table2array(Array(:,1)),'InputFormat','yyyy-MM-dd HH:mm:ss.SSSSSS');
msgID = Array(:, 2);
nodeID = Array(:, 3);
rssi = table2array(Array(:,4));
snr = Array(:, 5);
battery = Array(:,6);
dir = table2array(Array(:,7));

% xplot
figure();
scatter(t, dir);
hold on
grid on;
legend('RSSI');
xlabel("Time (s)");
ylabel("RSSI (dB)");