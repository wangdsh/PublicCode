import smtplib
from email.mime.text import MIMEText

def send_email(text):

    msg = MIMEText(text, 'plain', 'utf-8')

    # 输入Email地址和口令  email address and password
    from_addr = 'xxx@163.com'
    password = 'xxx'
    # 输入收件人地址  the receiver
    to_addr = 'xxx@163.com'
    # 输入SMTP服务器地址  SMTP server address
    smtp_server = 'smtp.163.com'

    server = smtplib.SMTP(smtp_server, 25) # SMTP协议默认端口是25  the default port of SMTP is 25
    # server.set_debuglevel(1)  # 给别人发邮件时容易被当成垃圾邮件，给自己发没事
    server.login(from_addr, password)
    server.sendmail(from_addr, [to_addr], msg.as_string())
    server.quit()

if __name__ == '__main__':
    send_email('This is a test.')

