package cmd

import (
  "os"
  "fmt"
  "errors"
  "reflect"
  "io/ioutil"
  "gopkg.in/yaml.v2"
  "github.com/spf13/cobra"
  "github.com/manifoldco/promptui"
)

type Config struct {
  ProjectName string `yaml:"projectName"`
  GeometryType string `yaml:"geometryType"`
}

var (
  cfg Config
  cfgFile string
  newCmd = &cobra.Command{
    Use:   "new",
    Short: "Create a game powered by WGE",
    Long:  `Initialize a project with either an interactive form or via a YAML configuration file where all needed game properties are specified.`,
    Run: func(cmd *cobra.Command, args []string) {
      if cfgFile != "" {
        parseCfgFile()
      } else {
        form()
      }
      showCfg()
    },
  }
)

func init() {
  newCmd.Flags().StringVarP(&cfgFile, "config", "c", "", "Use a predefined YAML config file")
}

func showCfg() {
  yamlData, err := yaml.Marshal(&cfg)
  if err != nil {
    fmt.Printf("[ERROR]: YAML marshaling failed %v\n", err)
    return
  }
  fmt.Println(string(yamlData))
}

func parseCfgFile() {
  _, err := os.Stat(cfgFile)
  if err != nil {
    fmt.Printf("[ERROR]: Config file does not exist (%v)\n", err)
    return
  }
  data, err := ioutil.ReadFile(cfgFile)
  if err != nil {
    fmt.Printf("[ERROR]: Config file could not be read (%v)\n", err)
    return
  }
  err = yaml.Unmarshal(data, &cfg)
  if err != nil {
    fmt.Printf("[ERROR]: Config file could not be deserialized (%v)\n", err)
    return
  }
  v := reflect.ValueOf(cfg)
  for i := 0; i < v.NumField(); i++ {
    fieldValue := v.Field(i).Interface()
    if fieldValue == "" {
      fmt.Printf("[ERROR]: Field '%s' in config file is empty\n", v.Type().Field(i).Name)
      return
    }
  }
}

func form() {
  form_projectName()
  form_geometryType()
}

func form_projectName() {
  validate := func(input string) error {
    if len(input) <= 1 {
      return errors.New("Project name must have more than 1 character")
    }
    return nil
  }
  prompt := promptui.Prompt{
    Label: "What is your project named?",
    Validate: validate,
  }
  result, err := prompt.Run()
  if err != nil {
    fmt.Printf("[ERROR]: Prompt failed %v\n", err)
    return
  }
  cfg.ProjectName = result
}

func form_geometryType() {
  prompt := promptui.Select{
    Label: "What type of geometry do you want to use?",
    Items: []string{"2D", "3D"},
  }
  _, result, err := prompt.Run()
  if err != nil {
    fmt.Printf("[ERROR]: Prompt failed %v\n", err)
    return
  }
  cfg.GeometryType = result
}
