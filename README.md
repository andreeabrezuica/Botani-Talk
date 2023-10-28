# **BotaniTalk**

This is a work-in-progress university project for _Interfaces and Peripherals_ course.

It is a smart plant box equipped with sensors for monitoring the soil humidity and ambiental light, and will try to help the plant grow healthy by automatically providing it with water and UV light when necessary.

It has an OLED screen that shows humidity, light and temperature information as well as a little face animation so you know the status of your plant at a first glance.

It can connect to the internet and store statistics data from the sensors and can notify you through e-mail if there's no water left in its tank or in case of other unhappy happenings.

## Proposed Features

- Collect data about soil humidity, temperature and ambiental light
- UV LEDs for when there is no natural light available
- Water tank and pump for when the soil gets dry
- Display screen for showing information
- E-mail notifications

## Guides and resources

### Some useful, genereal documentation

- [Git Explained in 100 Seconds](https://youtu.be/hwP7WQkmECE) (short video visualizing the bare basics of how the Git versioning system works)
- [Getting Started with Git and GitHub](https://github.com/git-guides) (a more comprehensive guide with the most common Git commands)
- [Markdown Documentation](https://docs.github.com/en/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax) (for updating `.md` files like this `README.md`)

### Software needed for contribution

- [Git](https://git-scm.com/download/win) (only command-line)
- _[optional]_ Any GUI for Git would be useful (you can still use the command-line if you want). [TortoiseGit](https://tortoisegit.org/) is a good option because it integrates in the Windows Shell
  > **Note:** If you want to use TortoiseGit, you need to install Git as well, as TortoiseGit is just a graphical interface for Git, it just writes and executes the Git commands for you.
- [Arduino IDE](https://www.arduino.cc/en/software) 2.2.1
