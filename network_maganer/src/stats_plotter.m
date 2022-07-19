close all;

Array=readtable('basic20.csv');

%t = datetime(Array(:, 1),'InputFormat','yyyy.MM.dd HH:mm:ss.SSS');
t = datetime(table2array(Array(:,1)),'InputFormat','yyyy-MM-dd HH:mm:ss.SSSSSS');
msgID = Array(:, 2);
nodeID = table2array(Array(:, 3));
rssi = table2array(Array(:,4));
snr = table2array(Array(:, 5));
battery = Array(:,6);
dir = table2array(Array(:,7));

% xplot
figure();
scatter(t, snr);
hold on
grid on;
legend('RSSI');
xlabel("Time (s)");
ylabel("RSSI (dB)");